#ifndef QB_LOCK_H
#define QB_LOCK_H

namespace qb_lock
{
    void lock_init(const char* src_path);
    void lock_uninit();
}

#endif
