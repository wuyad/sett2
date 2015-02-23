#include "runtime_class.h"
#include "stat_proc.h"
#include "classify_proc.h"
#include "db_storage.h"
#include "exclude_proc.h"
#include "file_storage.h"
#include "std_proc.h"
#include "null_proc.h"

runtime_class::map_runtime_class_t
runtime_class::all_proces_;

INIT_RUNTIME(stat_proc) 
INIT_RUNTIME(classify_proc) 
INIT_RUNTIME(db_storage) 
INIT_RUNTIME(exclude_proc) 
INIT_RUNTIME(file_storage) 
INIT_RUNTIME(std_proc) 
INIT_RUNTIME(null_proc) 


