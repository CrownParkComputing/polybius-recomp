/* POSIX file calls share guest mounts/FDs with kernel calls, but return -1 and guest errno. */
#undef NDEBUG
#include "../src/runtime_file.c"
#include <assert.h>

static _Thread_local int32_t guest_errno;
int32_t *runtime_errno(void) { return &guest_errno; }
int32_t runtime_guest_errno(int error) {
    switch (error) {
    case ENOENT:return 2; case EBADF:return 9; case EACCES:return 13;
    case EFAULT:return 14; case EINVAL:return 22; case EROFS:return 30;
    default:assert(!"unexpected host error");return 5;
    }
}
int tempest_skip_intro(void) { return 0; }
uint64_t runtime_wait_begin(void) { return 0; }
void runtime_wait_end(const char *what,uint64_t start,const char *detail,long long arg) {
    (void)what; (void)start; (void)detail; (void)arg;
}
void runtime_guest_backtrace(char *out,size_t size,int maximum) {
    (void)maximum; if (size) *out=0;
}
uintptr_t runtime_lookup(const RuntimeExport *table,size_t count,const char *name) {
    for (size_t i=0;i<count;++i) if (!strcmp(table[i].name,name)) return (uintptr_t)table[i].function;
    return 0;
}

int main(void) {
    char root[512];
    const char *tmp=getenv("TMPDIR");
    snprintf(root,sizeof(root),"%s/pacman-posix-file-XXXXXX",tmp && *tmp ? tmp : "/tmp");
    assert(mkdtemp(root));
    char app[512],user[512],asset[1024],saved[1024];
    snprintf(app,sizeof(app),"%s/app",root);
    snprintf(user,sizeof(user),"%s/user",root);
    assert(mkdir(app,0700)==0 && mkdir(user,0700)==0);
    snprintf(asset,sizeof(asset),"%s/asset.bin",app);
    snprintf(saved,sizeof(saved),"%s/roundtrip.bin",user);
    int host=open(asset,O_WRONLY|O_CREAT|O_EXCL,0600);
    assert(host>=0 && write(host,"app-data",8)==8 && close(host)==0);
    /* Force guest and host descriptor numbers apart. Passing a guest FD straight
     * to the host would read /dev/null or close one of these unrelated handles. */
    int occupied[8];
    for (int i=0;i<8;++i) { occupied[i]=open("/dev/null",O_RDONLY); assert(occupied[i]>=0); }
    runtime_file_configure(app,user);
    assert(runtime_file_mount("/temp0",user)==0);
    int32_t fd=posix_open("asset.bin",0,0);
    assert(fd>=3);
    char data[16]={0};
    assert(posix_read(fd,data,sizeof(data))==8 && !memcmp(data,"app-data",8));
    assert(posix_lseek(fd,0,SEEK_SET)==0);
    assert(posix_read(fd,data,3)==3 && !memcmp(data,"app",3));
    struct { uint64_t before; GuestStat value; uint64_t after; }
        st={UINT64_C(0x1122334455667788),{0},UINT64_C(0x8877665544332211)};
    assert(posix_fstat(fd,&st.value)==0 && st.value.size==8 && (st.value.mode&0170000)==0100000);
    assert(st.before==UINT64_C(0x1122334455667788) && st.after==UINT64_C(0x8877665544332211));
    assert(posix_stat("/hostapp/asset.bin",&st.value)==0 && st.value.size==8);
    assert(posix_close(fd)==0);
    for (int i=0;i<8;++i) assert(fcntl(occupied[i],F_GETFD)>=0);

    /* FreeBSD/PS4 open flags: O_RDWR=2, O_CREAT=0x200, O_TRUNC=0x400. */
    fd=posix_open("/temp0/roundtrip.bin",2|0x200|0x400,0600);
    assert(fd>=3);
    assert(posix_fcntl(fd,1,0)==0 && posix_fcntl(fd,2,1)==0 && posix_fcntl(fd,1,0)==1);
    assert(posix_fcntl(fd,3,0)==2);
    assert(posix_fcntl(fd,4,0x8|0x4)==0 && posix_fcntl(fd,3,0)==(2|0x8|0x4));
    assert(posix_fcntl(fd,4,0)==0 && posix_fcntl(fd,3,0)==2);
    assert(posix_fcntl(fd,1234,0)==-1 && guest_errno==22);
    const char payload[]="save payload";
    assert(posix_write(fd,payload,sizeof(payload))==(int64_t)sizeof(payload));
    assert(posix_lseek(fd,0,SEEK_SET)==0);
    memset(data,0,sizeof(data));
    assert(posix_read(fd,data,sizeof(data))==(int64_t)sizeof(payload));
    assert(!memcmp(data,payload,sizeof(payload)));
    assert(posix_stat("/temp0/roundtrip.bin",&st.value)==0 && st.value.size==(int64_t)sizeof(payload));
    int64_t distant=INT64_C(0x100000000)+7;
    assert(posix_lseek(fd,distant,SEEK_SET)==distant);
    assert(posix_lseek(fd,0,SEEK_CUR)==distant);
    assert(posix_lseek(fd,0,19)==-1 && guest_errno==22);
    assert(posix_fstat(fd,NULL)==-1 && guest_errno==14);
    assert(posix_ftruncate(fd,3)==0 && posix_fstat(fd,&st.value)==0 && st.value.size==3);
    assert(posix_fsync(fd)==0);
    assert(posix_mkdir("/temp0/child",0700)==0);
    assert(posix_stat("/temp0/child/.././roundtrip.bin",&st.value)==0 && st.value.size==3);
    assert(posix_stat("/temp0/child/../../roundtrip.bin",&st.value)==-1 && guest_errno==13);
    assert(posix_rmdir("/temp0/child")==0);
    assert(posix_close(fd)==0);
    assert(posix_fcntl(fd,1,0)==-1 && guest_errno==9);
    assert(posix_read(fd,data,1)==-1 && guest_errno==9);
    assert(posix_write(fd,data,1)==-1 && guest_errno==9);
    assert(posix_lseek(fd,0,SEEK_SET)==-1 && guest_errno==9);
    assert(posix_fstat(fd,&st.value)==-1 && guest_errno==9);
    assert(posix_close(fd)==-1 && guest_errno==9);
    assert(posix_open("/app0/absent.bin",0,0)==-1 && guest_errno==2);
    assert(posix_open("/app0/asset.bin",1,0)==-1 && guest_errno==30);
    assert(posix_open("/temp0/../asset.bin",0,0)==-1 && guest_errno==13);
    assert(posix_stat("/temp0/absent.bin",&st.value)==-1 && guest_errno==2);
    assert(posix_stat("/app0/asset.bin",NULL)==-1 && guest_errno==14);
    assert(posix_chmod("/app0/asset.bin",0600)==-1 && guest_errno==30);
    assert(posix_chmod("/temp0/roundtrip.bin",0640)==0);
    assert(posix_stat("/temp0/roundtrip.bin",&st.value)==0 && (st.value.mode&0777)==0640);
    assert(posix_mkdir("/temp0/subdir",0700)==0);
    assert(posix_rename("/temp0/roundtrip.bin","/temp0/subdir/moved.bin")==0);
    assert(posix_unlink("/temp0/subdir/moved.bin")==0);
    assert(posix_rmdir("/temp0/subdir")==0);
    assert(sce_open("/app0/absent.bin",0,0)==ERR(2));
    assert(sce_read(-1,data,1)==ERR(9));
    assert(runtime_file_resolve("open")== (uintptr_t)posix_open);
    assert(runtime_file_resolve("read")==runtime_file_resolve("_read"));
    assert(runtime_file_resolve("write")==runtime_file_resolve("_write"));
    for (int i=0;i<8;++i) assert(close(occupied[i])==0);
    assert(unlink(asset)==0 && access(saved,F_OK)==-1 && rmdir(app)==0 && rmdir(user)==0 && rmdir(root)==0);
    puts("POSIX file mounts, descriptor routing, roundtrip and errors passed");
    return 0;
}
