#ifndef _FS_NFSD_FAULT_INJECT_H
#define _FS_NFSD_FAULT_INJECT_H

#include <linux/sunrpc/svc.h>

#if defined(CONFIG_NFSD_NEW_FAULT_INJECTION)

extern void nfsd_fault_inject_init(void);
extern void nfsd_fault_inject_exit(void);

extern bool nfsd_fault_injection_major_id_take(struct svc_rqst *rqstp, char **pmajor_id);
extern void nfsd_fault_injection_major_id_release(bool activated);

#else

static inline void nfsd_fault_inject_init(void) {}
static inline void nfsd_fault_inject_exit(void) {}
static inline bool nfsd_fault_injection_major_id_take(struct svc_rqst *rqstp, char **pmajor_id) {
	(void)rqstp;
	(void)pmajor_id;
	return false;
}
static inline void nfsd_fault_injection_major_id_release(bool activated) { (void)activated; }

#endif

#endif /* _FS_NFSD_FAULT_INJECT_H */
