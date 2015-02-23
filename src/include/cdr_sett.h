#ifndef __CDR_SETT_H__
#define __CDR_SETT_H__

#ifdef _MSC_VER
	#pragma warning( disable : 4996 4819 )
#endif

#include <string>
#include <iosfwd>
#include <boost/variant.hpp>
#include <my_datetime.h>

#define MAX_TELNO_LEN 40

struct telno {
	char net_code[6];			   // ÍøºÅ
	char country_code[5];		   // ¹ú¼ÒÂë
	char area_code[5];			   // ÇøºÅ
	char user_code[21];				// ÓÃ»§ºÅÂë
};

enum CDR_TYPE {
	NORMAL_CDR,
	ERROR_CDR,
	REPEAT_CDR,
	DBFAIL
};

#define F_ANSWER_DATETIME               1
#define F_APPEND_FEE                    2  
#define F_AREA_CODE                     3  
#define F_BASE_FEE                      4  
#define F_BEGIN_DATETIME                5
#define F_BEGIN_DAY                     6  
#define F_BEGIN_HOUR                    7  
#define F_BEGIN_MINUTE                  8  
#define F_BEGIN_MONTH                   9  
#define F_BEGIN_SECOND                  10 
#define F_BEGIN_YEAR                    11 
#define F_CALLEDNO                      12 
#define F_CALLERNO                      13 
#define F_CHARGE_TERM_ID                14
#define F_CLS_AREA_TYPE                 15 
#define F_CLS_BUSI_TYPE                 16 
#define F_CLS_CALL_ORIENT               17
#define F_CLS_CALL_TRANS                18
#define F_CLS_CALLED_AC                 19 
#define F_CLS_CALLED_CARR               20
#define F_CLS_CALLED_IN_TYPE            21
#define F_CLS_CALLED_INTERNET           22
#define F_CLS_CALLED_IP_TYPE            23
#define F_CLS_CALLED_PREFIX             24
#define F_CLS_CALLED_SECT               25
#define F_CLS_CALLED_SP_ID              26
#define F_CLS_CALLED_SPCODE             27
#define F_CLS_CALLED_SSERV              28
#define F_CLS_CALLED_SSERV_NO           29
#define F_CLS_CALLED_TRUNK_CARR         30
#define F_CLS_CALLED_TTYPE              31
#define F_CLS_CALLER_AC                 32 
#define F_CLS_CALLER_CARR               33
#define F_CLS_CALLER_IN_TYPE            34
#define F_CLS_CALLER_INTERNET           35
#define F_CLS_CALLER_IP_TYPE            36
#define F_CLS_CALLER_PREFIX             37
#define F_CLS_CALLER_SECT               38
#define F_CLS_CALLER_SP_ID              39
#define F_CLS_CALLER_SPCODE             40
#define F_CLS_CALLER_SSERV              41
#define F_CLS_CALLER_SSERV_NO           42
#define F_CLS_CALLER_TRUNK_CARR         43
#define F_CLS_CALLER_TTYPE              44
#define F_CLS_CURRENCY                  45 
#define F_CLS_DEBIT                     46 
#define F_CLS_FEE                       47 
#define F_CLS_FEERATE_ID                48
#define F_CLS_INET_TOLL_TYPE            49
#define F_CLS_LOCAL_TYPE                50
#define F_CLS_SETT_CLSID                51
#define F_CLS_SETT_HOP                  52 
#define F_CLS_SETT_OBJ                  53 
#define F_CLS_SETT_OBJ_TYPE             54
#define F_CLS_SETT_PERIOD               55
#define F_CLS_SETT_TYPE                 56 
#define F_CLS_TOLL_TYPE                 57 
#define F_COLLECT_SOURCE                58
#define F_CONNECTED_NUMBER              59
#define F_COUNT                         60 
#define F_DESTNO                        61 
#define F_DONE_TIME                     62 
#define F_DURATION                      63 
#define F_END_DATETIME                  64 
#define F_END_DAY                       65 
#define F_END_HOUR                      66 
#define F_END_MINUTE                    67 
#define F_END_MONTH                     68 
#define F_END_SECOND                    69 
#define F_END_YEAR                      70 
#define F_ERROR                         71 
#define F_ERROR_CAUSE                   72 
#define F_FEE_CODE                      73 
#define F_FEE_FLAG                      74 
#define F_FEE_NO                        75 
#define F_FEE_NO_ATTR                   76 
#define F_FEE_TYPE                      77 
#define F_FEE_USER_FLAG                 78 
#define F_FWD_SMGW_NO                   79 
#define F_INCOMING_TRUNK                80
#define F_MESSAGE_ID                    81 
#define F_MONEY_TYPE                    82 
#define F_MSG_ID                        83 
#define F_MSG_LENGTH                    84 
#define F_OTHER_PARAM                   85 
#define F_OUTGOING_TRUNK                86
#define F_PRIORITY                      87 
#define F_RATE_TYPE                     88 
#define F_RECORD_TYPE                   89 
#define F_SERVICE_ID                    90 
#define F_SERVICE_TYPE                  91 
#define F_SM_TYPE                       92 
#define F_SMGW_NO                       93 
#define F_SMSC_NO                       94 
#define F_SMSG_ID                       95 
#define F_SOURCE_FILE                   96 
#define F_SOURCE_OFFSET                 97 
#define F_SP_ID                         98 
#define F_STATION_NO                    99 
#define F_STD_BEGIN_DATETIME            100
#define F_STD_CALLEDNO                  101
#define F_STD_CALLERNO                  102
#define F_STD_CHARGE_TERM_ID            103
#define F_STD_DESTNO                    104
#define F_STD_DURATION                  105
#define F_STREAM_NO                     106
#define F_SUB_TYPE                      107
#define F_SUM_FEE                       108
#define F_SUPPORT                       109
#define F_TYPE_OF_TRAFFIC               110
#define F_USER_ID                       111
#define F_SETT_OBJ_FLAG                 112
#define F_CLS_CALLER_COUNTRY	        113
#define F_CLS_CALLED_COUNTRY	        114
#define F_STD_END_DATETIME              115
#define F_CLS_TYPE_FLAG					116
#define F_CLS_STAT_DATE					117
#define F_CLS_SM_TYPE					118

#define F_FILETYPE                      200
#define F_FILESIZE                      201
#define F_FILENAME                      202
#define F_FILEPATH                      203
#define F_FULL_FILENAME                 204
#define F_RECORD_NUM                    205
#define F_RAW_FEE                       206
#define F_REPEAT                        207
#define F_PROC_TIME						208
#define F_USER_FLAG                     209		// 1-¶ªÆú

#define MAX_FIELD_COUNT                 255

typedef boost::variant<int, long long, std::string, wuya::datetime, telno> cdr_value_t;

class cdr_ex_base {
public:
	class cdr_ex_base_iterator {
	public:
		cdr_ex_base_iterator():ex_(0), index_(0) {
			while( index_<MAX_FIELD_COUNT && (!ex_->use_flag_[index_]) ) {
				++index_;
			}
		}
		cdr_ex_base_iterator(cdr_ex_base* ex, int index):ex_(ex), index_(index) {
			while( index_<MAX_FIELD_COUNT && (!ex_->use_flag_[index_]) ) {
				++index_;
			}
		}
		cdr_ex_base_iterator operator++(int) {
			cdr_ex_base_iterator tmp = *this;
			do {
				++index_;
			}while( index_<MAX_FIELD_COUNT && (!ex_->use_flag_[index_]) );
			return tmp;
		}
		cdr_ex_base_iterator& operator++() {
			do {
				++index_;
			}while( index_<MAX_FIELD_COUNT && (!ex_->use_flag_[index_]) );
			return *this;
		}
		cdr_ex_base_iterator operator--(int) {
			cdr_ex_base_iterator tmp = *this;
			do {
				--index_;
			}while( index_>=0 && (!ex_->use_flag_[index_]) );
			--index_;
			return tmp;
		}
		cdr_ex_base_iterator& operator--() {
			do {
				--index_;
			}while( index_>=0 && (!ex_->use_flag_[index_]) );
			return *this;
		}
		const cdr_ex_base_iterator operator++(int) const {
			cdr_ex_base_iterator tmp = *this;
			do {
				++index_;
			}while( index_<MAX_FIELD_COUNT && (!ex_->use_flag_[index_]) );
			return tmp;
		}
		const cdr_ex_base_iterator& operator++() const {
			do {
				++index_;
			}while( index_<MAX_FIELD_COUNT && (!ex_->use_flag_[index_]) );
			return *this;
		}
		const cdr_ex_base_iterator operator--(int) const {
			cdr_ex_base_iterator tmp = *this;
			do {
				--index_;
			}while( index_>=0 && (!ex_->use_flag_[index_]) );
			return tmp;
		}
		const cdr_ex_base_iterator& operator--() const {
			do {
				--index_;
			}while( index_>=0 && (!ex_->use_flag_[index_]) );
			return *this;
		}
		bool operator==(const cdr_ex_base_iterator& i) const {
			return index_ == i.index_ && ex_ == i.ex_;
		}
		bool operator!=(const cdr_ex_base_iterator& i) const {
			return index_ != i.index_ || ex_ != i.ex_;
		}
		cdr_value_t& operator*() {
			return ex_->value_[index_];
		}
		const cdr_value_t& operator*() const {
			return ex_->value_[index_];
		}
	private:
		mutable cdr_ex_base* ex_;
		mutable int index_;
	};
	friend class cdr_ex_base_iterator;

	typedef cdr_ex_base::cdr_ex_base_iterator iterator;
	typedef const cdr_ex_base::cdr_ex_base_iterator const_iterator;
	typedef cdr_value_t& reference;
	typedef const cdr_value_t& const_reference;

	cdr_ex_base() {
		clear();
	}

	void clear() {
		for( int i=0;i<MAX_FIELD_COUNT;++i ) {
			use_flag_[i] = false;
			value_[i] = 0;
		}
	}

	inline bool has(int field_index) const {
		return use_flag_[field_index];
	}
	
	cdr_value_t& get_variant(int field_index) {
		return value_[field_index];
	}

	const cdr_value_t& get_variant(int field_index) const{
		return value_[field_index];
	}

	template<class T>
	T& get(int field_index) {
		static T t = T();
		if( use_flag_[field_index] ) {
			try {
				return boost::get<T>(value_[field_index]);
			} catch( ... ) {
				return t;
			}
		} else {
			return t;
		}
	}

	template<class T>
	const T& get(int field_index) const {
		static T t = T();
		if( use_flag_[field_index] ) {
			try {
				return boost::get<T>(value_[field_index]);
			} catch( ... ) {
				return t;
			}
		} else {
			return t;
		}
	}

	template<class T>
	void set(int field_index, const T& value) {
		value_[field_index] = value;
		use_flag_[field_index] = true;
	}

	iterator begin() {
		return cdr_ex_base_iterator(this, 0);
	}

	iterator end() {
		return cdr_ex_base_iterator(this, MAX_FIELD_COUNT);
	}

	const_iterator begin() const {
		return cdr_ex_base_iterator((cdr_ex_base*)this, 0);
	}

	const_iterator end() const {
		return cdr_ex_base_iterator((cdr_ex_base*)this, MAX_FIELD_COUNT);
	}
protected:
	cdr_value_t value_[MAX_FIELD_COUNT];
	bool use_flag_[MAX_FIELD_COUNT];
};

template<>
inline int& cdr_ex_base::get<int>(int field_index) {
	static int t = -1;
	if( use_flag_[field_index] ) {
		try {
			return boost::get<int>(value_[field_index]);
		} catch( ... ) {
			return t;
		}
	} else {
		return t;
	}
}

template<>
inline long long& cdr_ex_base::get<long long>(int field_index) {
	static long long t = (long long)-1;
	if( use_flag_[field_index] ) {
		try {
			return boost::get<long long>(value_[field_index]);
		} catch( ... ) {
			return t;
		}
	} else {
		return t;
	}
}

template<>
inline const int& cdr_ex_base::get<int>(int field_index) const {
	static int t = -1;
	if( use_flag_[field_index] ) {
		try {
			return boost::get<int>(value_[field_index]);
		} catch( ... ) {
			return t;
		}
	} else {
		return t;
	}
}

template<>
inline const long long& cdr_ex_base::get<long long>(int field_index) const {
	static long long t = (long long)-1;
	if( use_flag_[field_index] ) {
		try {
			return boost::get<long long>(value_[field_index]);
		} catch( ... ) {
			return t;
		}
	} else {
		return t;
	}
}

struct cdr_ex : public cdr_ex_base {
	bool has_classified;
	cdr_ex():next(0),has_classified(false) {
	}
	cdr_ex* next;

	~cdr_ex() {
		while( next ) {
			cdr_ex* p = next->next;
			delete next;
			next = p;
		}
	}
	void clear() {
		cdr_ex_base::clear();
		next = 0;
		has_classified = false;
	}
};

inline std::ostream& operator<< (std::ostream& os, const telno& no) {
	char buf[MAX_TELNO_LEN];
	memset(buf,0, MAX_TELNO_LEN);
	int offset=0;
	size_t l = strlen(no.net_code);
	if( l!=0 ) {
		sprintf(buf+offset, "%s", no.net_code);
		offset += l;
	}
	l = strlen(no.country_code);
	if( l!=0 ) {
		sprintf(buf+offset, "%s", no.country_code);
		offset += l;
	}
	l = strlen(no.area_code);
	if( l!=0 ) {
		sprintf(buf+offset, "%s", no.area_code);
		offset += l;
	}
	l = strlen(no.user_code);
	if( l!=0 ) {
		sprintf(buf+offset, "%s", no.user_code);
		offset += l;
	}
	os << buf;
	return os;
}

inline std::ostream& operator<< (std::ostream& os, const cdr_ex& cdr) {
	const cdr_ex* p_cdr = &cdr;
	do {
		cdr_ex::const_iterator i = (*p_cdr).begin();
		for( ;i!=(*p_cdr).end();++i ) {
			os << (*i) << '\t';
		}
		os << std::endl;
	}while( (p_cdr=p_cdr->next)!=0 );
	return os;
}

inline bool operator<(const telno& no1, const telno& no2) {
	if( strcmp(no1.net_code, no2.net_code)<0 
		|| strcmp(no1.country_code, no2.country_code)<0 
		|| strcmp(no1.area_code, no2.area_code)<0 
		|| strcmp(no1.user_code, no2.user_code)<0 ) {
		return true;
	}
	return false;
}

inline bool operator==(const telno& no1, const telno& no2) {
	if( strcmp(no1.net_code, no2.net_code)==0 
		|| strcmp(no1.country_code, no2.country_code)==0 
		|| strcmp(no1.area_code, no2.area_code)==0 
		|| strcmp(no1.user_code, no2.user_code)==0 ) {
		return true;
	}
	return false;
}

#endif // __CDR_SETT_H__


