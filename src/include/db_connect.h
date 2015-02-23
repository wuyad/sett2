#ifndef __DB_CONNECT_H__
#define __DB_CONNECT_H__

#include <wuya/connect_pool.h>
#include <wuya/ace_ipc.h>

typedef wuya::conn_pool_t<wuya::ace_mutex, wuya::ace_condition> conn_pool;

class database {
public:
    database():db_(0) {
        db_ = conn_pool::instance()->get_connect();
    }
    ~database() {
        if( db_ ) {
            conn_pool::instance()->revert_connect(db_);
            db_=0;
        }
    }
    bool has_connect(){
        return db_!=0;
    }
    operator void*() {
        return(void*)db_;
    }
    operator otl_connect&() {
        return *db_;
    }
    otl_connect* operator ->() {
        return db_;
    }
private:
    otl_connect* db_;
};

#endif // __DB_CONNECT_H__



