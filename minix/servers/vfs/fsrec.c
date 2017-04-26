#include "fs.h"
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <minix/callnr.h>
#include <minix/safecopies.h>
#include <minix/endpoint.h>
#include <minix/com.h>
#include <minix/sysinfo.h>
#include <minix/u64.h>
#include <minix/vfsif.h>
#include <sys/ptrace.h>
#include <sys/svrctl.h>
#include <sys/resource.h>
#include <sys/dirent.h>
#include "file.h"
#include "vnode.h"
#include "vmnt.h"
#include "path.h"
#include "lock.h"

/* System call inodewalker */
int do_inodewalker(){
	printf("Start of inode walkeer from VFS\n");
	int r;
	struct vmnt *vmp;
	struct vnode *vp;
	struct lookup resolve;
	char fullpath[PATH_MAX] = "/";

	/* Get a virtual inode and virtual mount corresponding to the path */
	lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
	resolve.l_vmnt_lock = VMNT_READ;
	resolve.l_vnode_lock = VNODE_READ;
	if ((vp = last_dir(&resolve, fp)) == NULL) return(err_code);

	/* Emit a request to FS */
	r = req_inodewalker(vmp->m_fs_e);

	/* Unlock virtual inode and virtual mount */
	unlock_vnode(vp);
	unlock_vmnt(vmp);
	put_vnode(vp);
	return r;
}

int do_znodewalker(){
        printf("Start of znode walkeer from VFS\n");
        int r;
        struct vmnt *vmp;
        struct vnode *vp;
        struct lookup resolve;
        char fullpath[PATH_MAX] = "/";

        /* Get a virtual inode and virtual mount corresponding to the path */
        lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
        resolve.l_vmnt_lock = VMNT_READ;
        resolve.l_vnode_lock = VNODE_READ;
        if ((vp = last_dir(&resolve, fp)) == NULL) return(err_code);

        /* Emit a request to FS */
        r = req_znodewalker(vmp->m_fs_e);

        /* Unlock virtual inode and virtual mount */
        unlock_vnode(vp);
        unlock_vmnt(vmp);
        put_vnode(vp);
        return r;
}

int do_zoneinfo(){

	dev_t device = job_m_in.m_fs_vfs_lookup.device;
	ino_t inode = job_m_in.m_fs_vfs_lookup.inode;
	
	printf("Start of zone info from VFS\n");
        int r;
        struct vmnt *vmp;
        struct vnode *vp;
        struct lookup resolve;
        char fullpath[PATH_MAX] = "/";

        /* Get a virtual inode and virtual mount corresponding to the path */
        lookup_init(&resolve, fullpath, PATH_NOFLAGS, &vmp, &vp);
        resolve.l_vmnt_lock = VMNT_READ;
        resolve.l_vnode_lock = VNODE_READ;
        if ((vp = last_dir(&resolve, fp)) == NULL) return(err_code);

        /* Emit a request to FS */
        r = req_zoneinfo(vmp->m_fs_e,device,inode);
	
	unlock_vnode(vp);
	unlock_vmnt(vmp);
	put_vnode(vp);

	return r;
}
