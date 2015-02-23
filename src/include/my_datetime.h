#ifndef __MY_DATETIME_H__
#define __MY_DATETIME_H__

#include <wuya/datetime.h>

namespace wuya {
	inline std::ostream& operator<< (std::ostream& os, const wuya::datetime& t) {
		os << t.date_time_str2();
		return os;
	}
}
#endif // __MY_DATETIME_H__

