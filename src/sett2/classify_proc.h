#ifndef __CLASSIFY_PROC_H__
#define __CLASSIFY_PROC_H__

#include "proc_base.h"

namespace SETT {
	enum SETT_TYPE {
		INET_SETT=1/*网间结算*/,
		TOLL_SETT/*网内长途结算*/,
		SP_SETT/*合作方结算*/,
	};

	enum BUSI_TYPE {
		VOICE=1/*普通语音*/,
		IPP/*IP 业务*/,
		INN/*智能网业务*/,
		IVR/*语音增值业务*/,
		SMSVAS/*短信增值业务*/,
		INTERNET/*互联网业务*/,
		SMSP2P/*网间短信*/,
	};

	enum AREA_TYPE {
		LOCAL_CALL=1/*本地*/,
		TOLL/*长途*/,
		INTERNATIONAL/*国际*/,
	};

	enum TTYPE {
		FIX=0/*固定电话*/,
		PHS=2/*小灵通*/,
		M800/*卫通800兆*/,
		GSM/*GSM手机*/,
		CDMA/*CDMA手机*/,
	};

	enum CALL_ORIENT {
		COME=1/*来*/,
		GO/*去*/,
	};

	enum SSERV_TYPE {
		NSSERV=0/*非特服*/,
		PAYSSERV=1/*收费特服*/,
		FREESSERV=2/*免费特服*/,
		NETSSERV=3/*上网号*/,
		EMERGENCY=4/*紧急特服*/,
	};

	enum DEBIT_TYPE {
		DEBIT=1/*借*/,
		CREDIT/*贷*/,
	};

	enum SETT_OBJECT_TYPE {
		ALL_OBJECT=0/*所有运营商*/,
		OTHER=1/*其他运营商*/,
		INNERNET/*网内其他省公司*/,
		SP_OBJECT/*SP*/,
	};

	enum IN_TYPE {
		IN_200=1/*200*/,
		IN_300/*300*/,
		IN_400/*400*/,
		IN_600/*600*/,
		IN_800/*800*/,
		IN_201/*201*/,
	};

	enum IP_TYPE {
		IP_CARD=0/*IP卡业务*/,
		IP_DIRECT/*IP直拨*/,
		TOLL_DIRECT/*长途直拨*/,
	};

	enum LOCAL_TYPE {
		INNER_SECT=1/*营业区内*/,
		INTER_SECT0/*营业区间(不通过区间线路)*/,
		INTER_SECT/*营业区间(通过区间线路)*/,
		UNKNOW_SECT/*未知*/,
	};

	enum TOLL_TYPE {
		COME_USE_MINE=1/*来向本网长途*/,
		GO_USE_MINE/*去向本网长途*/,
		COME_USE_OTHER/*来向异网长途*/,
		GO_USE_OTHER/*去向异网长途*/,
	};

	enum INET_TOLL_TYPE {
		COME_GAT=1/*来向港澳台*/,
		COME_NGAT/*来向其他国际*/,
		GO_GAT/*去向港澳台*/,
		GO_NGAT/*去向其他国际*/,
	};

	enum CALL_TRANS {
		CALL_TRANS_NO=1/*非呼叫转移*/,
		CALL_TRANS_YES=2/*呼叫转移*/,
	};

	enum {
		NONE=-1
	};
}

struct call_info {
	int carr;
	int ac;
	int sect;
	int ttype;
	int sserv_type;
	int sserv_no;
	int prefix;
	int ip_type;
	int in_type;
	int trunk_carr;
	int spcode;
	int sp_id;
	int internet;
	int country;
	int is_call_nav;//是否电话导航
	int is_center_sect;
	int trunk_type;// 0本地, 1长途, 2混群

	call_info():carr(-1),ac(-1),sect(-1),ttype(-1),sserv_type(-1),sserv_no(-1),prefix(-1),
	ip_type(-1),in_type(-1),trunk_carr(-1),spcode(-1),sp_id(-1),internet(-1),country(-1),
		is_call_nav(0),is_center_sect(0), trunk_type(-1)  {
	}
};

struct feerate_info;

class classify_proc : public proc_base {
public:
	classify_proc():sett_type_(-1),sett_obj_type_(-1),sett_obj_(-1),p_cdr_(0), p_ctx_(0),
		busi_type_(-1),area_type_(-1),local_type_(-1),toll_type_(-1),inet_toll_type_(-1),
		call_trans_(-1),call_orient_(-1),sett_clsid_(-1),feerate_id_(-1),fee_(0),
		sett_hop_(0),currency_(-1),debit_(-1),sett_obj_flag_(-1){
	}
protected:
	bool proc_record(cdr_ex& cdr, proc_context& ctx);
	bool classify();
	void clear();
private:
	cdr_ex* p_cdr_; 
	proc_context* p_ctx_;
	cdr_ex& get_cdr(){
		return *p_cdr_;
	}
	proc_context& get_ctx(){
		return *p_ctx_;
	}
	void guest_collect_source();
	void set_error_n(int err_no, ...);
	void set_error(int err_no);
	void store_to_cdr();
	void store_to_cdr2();

	int fit_condition(int key);

	void parse_sett_type();
	void parse_call_info(bool is_caller);
	void parse_busi_type();
	void parse_area_type();
	void parse_sett_period();

	void compu_fee();
	long long get_group_fee(int id);
	long long get_singal_fee(const feerate_info* feerate);
	long long get_singal_group_fee(const feerate_info* feerate, int value);
	long long get_ts_fee_total(int id);
	int get_ts_duration(const wuya::datetime& begin, int duratin, int hour, int hour2);

	bool valid_201();

		
	std::string collect_source_;
	// 结算要素
	int sett_type_;
	int busi_type_;
	call_info caller_;
	call_info called_;
	int area_type_;
	int local_type_;
	int toll_type_;
	int inet_toll_type_;
	int call_trans_;
	int call_orient_;
	// 其他结算信息
	int sett_obj_;
	int sett_obj_type_;
	int sett_clsid_;
	int feerate_id_;
	long long sett_hop_;
	long long fee_;
	int currency_; 
	int debit_;
	int sett_obj_flag_;
	std::string sett_period_; 
};
#endif // __CLASSIFY_PROC_H__
