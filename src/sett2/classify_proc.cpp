#include "classify_proc.h"
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include "data_finder.h"
#include "sett_condi.h"
#include "sett_period.h"
#include <stdio.h>
#include <stdarg.h>
#include <wuya/timespan.h>

using namespace std;
using namespace boost;

#define MAX_ERROR_LEN 128

const char* err_code[] = {
	/*0*/"OK",
	/*1*/"more than two condition fited (%d, %d).",
	/*2*/"no fitted condition",
	/*3*/"no matched feerate (%d)",
	/*4*/"not valid 201 cdr",
	/*5*/"can not find settle object",
	/*6*/"IP card business do not settle",
	/*7*/"cnc use other IP business do not settle",
	/*8*/"",
};

bool classify_proc::proc_record(cdr_ex& cdr, proc_context& ctx) {
	clear();
	p_cdr_ = &cdr;
	p_ctx_ = &ctx;

	collect_source_ = get_cdr().get<string>(F_COLLECT_SOURCE);
	if( collect_source_=="" ) {
		guest_collect_source();
	}
	bool ret=true;
	if( !classify() ) {
		ret = false;
		store_to_cdr();
		store_to_cdr2();
	} else {
		store_to_cdr();
		// 判断并设置结算分类标识，如果多个条件都满足，则产生错单
		int fit_count = 0;
		int ret = 0;
		int key = sett_obj_*100+sett_type_*10+sett_obj_type_;
		ret=fit_condition(key);
		if( ret==-1 ) {
			key = SETT::ALL_OBJECT*100+sett_type_*10+0;
			ret = fit_condition(key);
		}
		if( ret==-1 ) {
			set_error(2);
			ret = false;
		} else if( ret==0 ) {
			ret = true;
		} else {
			ret = false;
		}
		store_to_cdr2();
	}

	return ret;
}

void classify_proc::clear() {
	sett_type_=-1;
	sett_obj_type_=-1;
	sett_obj_=-1;
	p_cdr_=0;
	p_ctx_=0;

	busi_type_=-1;
	area_type_=-1;
	local_type_=-1;
	toll_type_=-1;
	inet_toll_type_=-1;

	call_trans_=-1;
	call_orient_=-1;
	sett_clsid_=-1;
	feerate_id_=-1;
	fee_=0;
	sett_hop_=0;
	currency_=-1;
	debit_=-1;
	sett_obj_flag_ = -1;

	caller_.carr=-1;
	caller_.ac=-1;
	caller_.sect=-1;
	caller_.ttype=-1;
	caller_.sserv_type=-1;
	caller_.sserv_no=-1;
	caller_.prefix=-1;
	caller_.ip_type=-1;
	caller_.in_type=-1;
	caller_.trunk_carr=-1;
	caller_.spcode=-1;
	caller_.sp_id=-1;
	caller_.internet=-1;
	caller_.country=-1;
	caller_.is_call_nav = 0;
	caller_.is_center_sect = 0;
	caller_.trunk_type=-1;

	called_.carr=-1;
	called_.ac=-1;
	called_.sect=-1;
	called_.ttype=-1;
	called_.sserv_type=-1;
	called_.sserv_no=-1;
	called_.prefix=-1;
	called_.ip_type=-1;
	called_.in_type=-1;
	called_.trunk_carr=-1;
	called_.spcode=-1;
	called_.sp_id=-1;
	called_.internet=-1;
	called_.country=-1;
	called_.is_call_nav = 0;
	called_.is_center_sect = 0;
	called_.trunk_type=-1;

	collect_source_="";
	sett_period_="";
}

void classify_proc::guest_collect_source() {
	string filetype = get_cdr().get<string>(F_FILETYPE);
	if( filetype=="S1240V74" ) {
		collect_source_ = "TOLL";
	} else if( filetype=="S1240CHB2" ) {
		collect_source_ = "TOLL";
	} else if( filetype=="NET2NET" ) {
		collect_source_ = "gw3";
	} else if( filetype=="201" ) {
		collect_source_ = "201";
	} else if( filetype=="IVR" ) {
		collect_source_ = "IVR";
	} else if( filetype=="SMS" ) {
		collect_source_ = "SMSC";
	} else if( filetype=="NGN" ) {
		collect_source_ = "NGN";
	} else if( filetype=="INET" ) {
		collect_source_ = "INET";
	}
	get_cdr().set(F_COLLECT_SOURCE, collect_source_);
}

int classify_proc::fit_condition(int key) {
	const multimap<int, classify_info>& sett_cls = data_finder::get_sett_classify();
	typedef pair<multimap<int, classify_info>::const_iterator, multimap<int, classify_info>::const_iterator> pair_tmp_t;
	pair_tmp_t sett_cls_fit = sett_cls.equal_range(key);
	multimap<int, classify_info>::const_iterator i = sett_cls_fit.first;
	int condi_id = 0;
	int fit_count = 0;

	int cls_id;
	int feerate_id;
	int debit_flag;

	for( ;i!=sett_cls_fit.second;++i ) {
		if( SETT_CONDI_INS->fit((*i).second.condi_id, get_cdr()) ) {
			if( fit_count>0 ) {
				// 错单
				set_error_n(1, condi_id, (*i).second.condi_id);
				return 1; // 多于一个匹配
			}
			++fit_count;
			cls_id = (*i).second.id;
			feerate_id = (*i).second.feerate_id;
			debit_flag = (*i).second.debit_flag;
			condi_id = (*i).second.condi_id;
		}
	}
	if( fit_count==0 ) {
		return -1; // 没有匹配
	}

	sett_clsid_ = cls_id;
	feerate_id_ = feerate_id;
	if( data_finder::get_feerate(feerate_id) == 0 ) {
		// 错单
		set_error_n(3, feerate_id);
		return false;
	}
	debit_ = SETT_CONDI_INS->eval(debit_flag, get_cdr());
	// 计算费用
	compu_fee();

	return 0; // 正常
}

void classify_proc::set_error_n(int err_no, ...) {
	get_cdr().set(F_ERROR, err_no);
	char buf[MAX_ERROR_LEN];

	va_list va;
	va_start(va, err_no);
	vsprintf(buf, err_code[err_no], va);
	va_end(va);

	get_cdr().set(F_ERROR_CAUSE, string(buf));
}

void classify_proc::set_error(int err_no) {
	get_cdr().set(F_ERROR, err_no);
	get_cdr().set(F_ERROR_CAUSE, string(err_code[err_no]));
}


bool classify_proc::classify() {
	if( get_cdr().has_classified ) {
		return true;
	}
	get_cdr().has_classified = true;
	parse_call_info(true);
	parse_call_info(false);
	if( collect_source_=="201" && !valid_201() ) {
		set_error(4);
		return false;
	}
	parse_busi_type();
	parse_sett_period();
	parse_sett_type();
	parse_area_type();
	if( busi_type_ == SETT::IPP && 
		(caller_.ip_type==SETT::IP_CARD || called_.ip_type==SETT::IP_CARD) ) {
		set_error(6);
		return false;
	}
	if( busi_type_ == SETT::IPP && called_.ip_type==SETT::IP_DIRECT ) {
		set_error(7);
		return false;
	}
	if( sett_obj_==-1 || sett_obj_==data_finder::get_cur_carr_id() ) {
		set_error(5);
		return false;
	}

	return true;
}

void classify_proc::parse_sett_type() {
	if( collect_source_=="gw3" || collect_source_=="gw6" || collect_source_=="201" ) {
		sett_type_ = SETT::INET_SETT;
	} else if( collect_source_=="TOLL" || collect_source_=="NGN" ) {
		sett_type_ = SETT::TOLL_SETT;
	} else if( collect_source_=="IVR" || collect_source_=="INET" ) {
		sett_type_ = SETT::SP_SETT;
	} else if( collect_source_=="SMSC" ) {
		if( busi_type_ == SETT::SMSVAS ) {
			sett_type_ = SETT::SP_SETT;
		} else {
			sett_type_ = SETT::INET_SETT;
		}
	}
}

void classify_proc::parse_busi_type() {
	if( collect_source_=="201" ) {
		busi_type_ = SETT::INN;
		return;
	}
	if( collect_source_ == "IVR" ) {
		busi_type_ = SETT::IVR;
		return;
	}
	if( collect_source_ == "INET" ) {
		busi_type_ = SETT::INTERNET;
		return;
	}
	// 短信网间
	if( collect_source_ == "SMSC" ) {
		if( busi_type_==-1 ) {
			busi_type_ = SETT::SMSP2P;
		}
	}
	if( busi_type_==-1 && (collect_source_ == "gw3" ||
						   collect_source_ == "gw6" ||
						   collect_source_ == "TOLL" ||
						   collect_source_ == "NGN") ) {
		busi_type_ = SETT::VOICE;
	}
}

void classify_proc::parse_call_info(bool is_caller) {
	telno* p_tno;
	call_info* p_call_info;
	if( is_caller ) {
		// 主叫号码
		if( !get_cdr().has(F_STD_CALLERNO) ) {
			return;
		}
		p_tno = &get_cdr().get<telno>(F_STD_CALLERNO);
		p_call_info = &caller_;
	} else {
		// 被叫号码
		if( !get_cdr().has(F_STD_CALLEDNO) ) {
			return;
		}
		p_tno = &get_cdr().get<telno>(F_STD_CALLEDNO);
		p_call_info = &called_;
	}
	telno& tno = *p_tno;
	call_info& call = *p_call_info;
	// 特服，含智能网
	int temp_ac = atoi(tno.area_code);
	if( temp_ac == data_finder::get_cur_ac() || temp_ac == 0 ) {
		const service_telno_info_t::value_type* ret = data_finder::get_special_number(tno.user_code);
		if( ret != 0 ) {
			call.carr = ret->second.carrier_id;
			if( ret->second.servcietype==5 || ret->second.servcietype==6 ) {
				// 智能网
				const string& no = ret->first;
				if( no.find("200")!=string::npos ) {
					call.in_type=SETT::IN_200;
				} else if( no.find("300")!=string::npos ) {
					call.in_type=SETT::IN_300;
				} else if( no.find("400")!=string::npos ) {
					call.in_type=SETT::IN_400;
				} else if( no.find("600")!=string::npos ) {
					call.in_type=SETT::IN_600;
				} else if( no.find("800")!=string::npos ) {
					call.in_type=SETT::IN_800;
				}
				busi_type_ = SETT::INN;
			} else {
				// 特服的被叫运营商
				call.sserv_no = atoi(ret->first.c_str());
				call.sserv_type = ret->second.servcietype;
			}
			call.ttype = SETT::FIX;
			// 区号
			call.ac = data_finder::get_cur_ac();
			call.sect = 1; // 市区
		} else {
			call.sserv_type = SETT::NSSERV;
		}
	} else {
		call.sserv_type = SETT::NSSERV;
	}
	// IP业务或长途选网
	const cicno_info_t::value_type* cic;
	if( (cic=data_finder::get_cicno_info(tno))!=0 ) {
		call.carr = cic->second.carrier_id;
		call.prefix = atoi(cic->second.no.c_str());
		call.ip_type = cic->second.type;
		if( cic->second.type == 0 || cic->second.type == 1 ) {
			//IP业务
			busi_type_ = SETT::IPP;
		} else { // 长途前缀
			busi_type_ = SETT::VOICE;
		} 
	}
	// 短信业务或语音增值
	const spcode_info* sp;
	if( (sp=data_finder::get_sp_id(tno.user_code))!=0 ) {
		// 短信增值
		call.sp_id = sp->id;
		call.spcode = atoi(tno.user_code);
		if( sp->type==0 ) {
			busi_type_ = SETT::SMSVAS;
		} else {
			busi_type_ = SETT::IVR;
		}
	}
	if( call.sserv_type == SETT::NSSERV ) {
		// 终端类型
		const hcode_info* hcode = data_finder::get_mobile_info(tno.user_code);
		if( hcode!= 0 ) {
			// 移动的主叫运营商与主叫区号在此填写
			if( call.carr==-1 )	call.carr = hcode->carrier_id;
			call.ac = atoi(hcode->dis_no.c_str());
			const cell_prefix_info* p = data_finder::get_cell_prefix_info(tno.user_code);
			if( p ) {
				if( p->net_type == 0 ) {
					call.ttype = SETT::GSM;
				} else {
					call.ttype = SETT::CDMA;
				}
			} else {
				// 如果区分不开，则认为是GSM
				call.ttype = SETT::GSM;
			}
		} else {
			// 根据号码前缀作粗判断
			const cell_prefix_info* p = data_finder::get_cell_prefix_info(tno.user_code);
			if( p ) {
				if( call.carr == -1 ) {
					call.carr = p->carrier_id;
				}
				if( p->net_type == 0 ) {
					call.ttype = SETT::GSM;
				} else {
					call.ttype = SETT::CDMA;
				}
			} else {
				// 不是移动电话
				const telno_segment_info* p = data_finder::get_fix_term_info(tno);
				if( p != 0 ) {
					if( call.carr==-1 )	call.carr = p->carrier_id;
					call.sect = p->sect_id;
					call.ac = atoi(p->areacode.c_str());
					call.ttype = p->term_type;
				}
			}
		}
	}
	// 中继运营商
	if( get_cdr().has(is_caller?F_INCOMING_TRUNK:F_OUTGOING_TRUNK) ) {
		string& trunk = get_cdr().get<string>(is_caller?F_INCOMING_TRUNK:F_OUTGOING_TRUNK);
		const trunk_info* tmp_trunk_info = data_finder::get_trunk_info(0, collect_source_.c_str(),
																	   trunk.c_str());
		if( tmp_trunk_info!=0 ) {
			call.trunk_carr = tmp_trunk_info->carrier_id;
			call.trunk_type = tmp_trunk_info->type;
		}
	}
	// 联通固网
	if( call.trunk_carr==4 && call.ttype==SETT::FIX ) {
		call.trunk_carr=14;
	}
	// 国家码
	call.country = atoi(tno.country_code);
	// 区号
	if( call.ac==-1 ) {
		call.ac = atoi(tno.area_code);
		if( call.ac==0 ) {
			call.ac=-1;
		}
	}
	// 判断是否中心营业区
	if( call.sect!=-1 ) {
		call.is_center_sect = data_finder::is_inner_city(call.sect)?1:0;
	}
	// 判断是否电话导航
	if( call.sserv_type!=SETT::NSSERV ) {
		if( call.carr == 1 ) { // 网通
			if( strcmp(tno.user_code, "114")==0 || 
				strcmp(tno.user_code, "116114")==0 ) {
				call.is_call_nav = 1;
			}
		} else if( call.carr == 2 ) { // 电信
			if( strcmp(tno.user_code, "114")==0 || 
				strcmp(tno.user_code, "118114")==0 ) {
				call.is_call_nav = 1;
			}
		} else if( call.carr == 3 ) { // 移动
			if( strcmp(tno.user_code, "12580")==0 ) {
				call.is_call_nav = 1;
			}
		} else if( call.carr == 4 ) { // 联通
			if( strcmp(tno.user_code, "10191")==0 || 
				strcmp(tno.user_code, "10198")==0 ) {
				call.is_call_nav = 1;
			}
		}
	}
}

void classify_proc::parse_area_type() {
	int me = data_finder::get_cur_carr_id();
	int local = data_finder::get_cur_ac();

	// 通话方向
	if( caller_.carr == me && caller_.ac==local ) {
		call_orient_ = SETT::GO;
	} else {
		call_orient_ = SETT::COME;
	}
	// 结算对象类型
	sett_obj_type_ = sett_type_;
	// 涉及三方则为呼转(两端皆为其他运营商 不算呼转)
//  int carr_count=0;
	map<int, int> carr_ids;
//  int last_carr=0;
	if( caller_.carr!=-1 && caller_.carr!=me ) {
//  	++carr_count;
		++carr_ids[caller_.carr];
	}
//  last_carr = caller_.carr;

	if( caller_.trunk_carr!=-1 && caller_.trunk_carr!=me /*&& caller_.trunk_carr!=last_carr*/ ) {
//  	++carr_count;
		++carr_ids[caller_.trunk_carr];
	}
//  last_carr = caller_.trunk_carr;

	if( called_.trunk_carr!=-1 && called_.trunk_carr!=me /*&& called_.trunk_carr!=last_carr*/ ) {
//  	++carr_count;
		++carr_ids[called_.trunk_carr];
	}
//  last_carr = called_.trunk_carr;

	if( called_.carr!=-1 && called_.carr!=me /*&& called_.carr!=last_carr*/ ) {
//  	++carr_count;
		++carr_ids[called_.carr];
	}

	if( carr_ids.size()>=2 ) {// 除了自己之外还有两方
		call_trans_ = SETT::CALL_TRANS_YES;
	} else {
		call_trans_ = SETT::CALL_TRANS_NO;
	}

	if( (caller_.country!=0 && caller_.country!=86)
		|| (called_.country!=0 && called_.country!=86) ) {
		// 国际
		area_type_ = SETT::INTERNATIONAL;
		bool is_gat = false;
		if( caller_.country==852||caller_.country==853||caller_.country==886
			|| called_.country==852||called_.country==853||called_.country==886 ) {
			is_gat = true;
		}
		if( called_.trunk_carr!=me ) { // 去向
			inet_toll_type_ = is_gat?SETT::GO_GAT:SETT::GO_NGAT;
			sett_obj_ = called_.trunk_carr;
			sett_obj_flag_ = 1;
		} else if( caller_.trunk_carr!=me ) { // 来向
			inet_toll_type_ = is_gat?SETT::COME_GAT:SETT::COME_NGAT;
			sett_obj_ = caller_.trunk_carr;
			sett_obj_flag_ = 0;
		}
	} else {
		if( busi_type_ == SETT::IPP && /*IP*/
			(busi_type_==SETT::VOICE && (caller_.ip_type==SETT::TOLL_DIRECT || /*长途直拔*/
										 called_.ip_type==SETT::TOLL_DIRECT))
		  ) {
			area_type_ =  SETT::TOLL;
			if( caller_.ip_type!=-1 ) {
				if( caller_.carr!=me ) {
					// 来向异网长途
					toll_type_ = SETT::COME_USE_OTHER;
					sett_obj_ = caller_.carr;
					sett_obj_flag_ = 0;
				} else {
					// 来向本网长途
					toll_type_ = SETT::COME_USE_MINE;
					sett_obj_ = called_.carr;
					sett_obj_flag_ = 1;
				}
			} else if( called_.ip_type!=-1 ) {
				if( called_.carr!=me ) {
					// 去向异网长途
					toll_type_ = SETT::GO_USE_OTHER;
					sett_obj_ = called_.carr;
					sett_obj_flag_ = 1;
				} else {
					// 去向本网长途
					toll_type_ = SETT::GO_USE_MINE;
					sett_obj_ = caller_.carr;
					sett_obj_flag_ = 0;
				}
			}
		} else if( caller_.trunk_type==1 || called_.trunk_type==1 ) { /*按中继类型判断长途*/
			area_type_ =  SETT::TOLL;
			if( caller_.trunk_type==1 ) {
				if( caller_.trunk_carr==me ) {
					// 来向本网长途
					toll_type_ = SETT::COME_USE_MINE;
					sett_obj_ = called_.trunk_carr;
					sett_obj_flag_ = 1;
				} else {
					// 来向异网长途
					toll_type_ = SETT::COME_USE_OTHER;
					sett_obj_ = caller_.trunk_carr;
					sett_obj_flag_ = 0;
				}
			} else { /*called_.trunk_type==1*/
				if( called_.trunk_carr==me ) {
					// 去向本网长途
					toll_type_ = SETT::GO_USE_MINE;
					sett_obj_ = caller_.trunk_carr;
					sett_obj_flag_ = 0;
				} else {
					// 去向异网长途
					toll_type_ = SETT::GO_USE_OTHER;
					sett_obj_ = called_.trunk_carr;
					sett_obj_flag_ = 1;
				}
			}
		} else if( !data_finder::is_local_ac(caller_.ac) ||	/*按号码判断长途*/
				   !data_finder::is_local_ac(called_.ac) ) {
			area_type_ =  SETT::TOLL;
			if( caller_.ac!=local && caller_.trunk_carr==me ) {
				// 来向本网长途
				toll_type_ = SETT::COME_USE_MINE;
				sett_obj_ = called_.trunk_carr;
				sett_obj_flag_ = 1;
			} else if( called_.ac!=local && called_.trunk_carr==me ) {
				// 去向本网长途
				toll_type_ = SETT::GO_USE_MINE;
				sett_obj_ = caller_.trunk_carr;
				sett_obj_flag_ = 0;
			} else if( caller_.ac!=local && caller_.trunk_carr!=me ) {
				// 来向异网长途
				toll_type_ = SETT::COME_USE_OTHER;
				sett_obj_ = caller_.trunk_carr;
				sett_obj_flag_ = 0;
			} else if( called_.ac!=local && called_.trunk_carr!=me ) {
				// 去向异网长途
				toll_type_ = SETT::GO_USE_OTHER;
				sett_obj_ = called_.trunk_carr;
				sett_obj_flag_ = 1;
			}
		} else {
			area_type_ = SETT::LOCAL_CALL;

			if( caller_.carr==-1 || caller_.sect==-1 || 
				called_.carr==-1 || called_.sect==-1 ) {
				local_type_ = SETT::UNKNOW_SECT;
			} else {
				int ret = data_finder::is_intersection(caller_.carr, caller_.sect, 
													   called_.carr, called_.sect);
				if( ret==0 ) {
					local_type_ = SETT::INNER_SECT;	// 区内
				} else if( ret == 2 ) {
					local_type_ = SETT::INTER_SECT0;// 区间（不经电路）
				} else if( ret==1 ) {
					local_type_ = SETT::INTER_SECT;	// 区间（经电路）
				} else {
					local_type_ = SETT::UNKNOW_SECT;
				}
			}
			if( caller_.trunk_carr!=-1 && caller_.trunk_carr!=me ) {
				sett_obj_ = caller_.trunk_carr;
				sett_obj_flag_ = 0;
			} else if( called_.trunk_carr!=-1 && called_.trunk_carr!=me ) {
				sett_obj_ = called_.trunk_carr;
				sett_obj_flag_ = 1;
			} else if( caller_.carr!=-1 && caller_.carr!=me ) {
				sett_obj_ = caller_.carr;
				sett_obj_flag_ = 0;
			} else {
				sett_obj_ = called_.carr;
				sett_obj_flag_ = 1;
			}
		}
	}
}

bool classify_proc::valid_201() {
	return get_cdr().get<string>(F_SERVICE_TYPE)=="2301";
}

void classify_proc::store_to_cdr() {
	sett_type_!=-1?get_cdr().set(F_CLS_SETT_TYPE, sett_type_):void(0);
	busi_type_!=-1?get_cdr().set(F_CLS_BUSI_TYPE, busi_type_):void(0);

	caller_.carr!=-1?get_cdr().set(F_CLS_CALLER_CARR, caller_.carr):void(0);
	caller_.ac!=-1?get_cdr().set(F_CLS_CALLER_AC, caller_.ac):void(0);
	caller_.sect!=-1?get_cdr().set(F_CLS_CALLER_SECT, caller_.sect):void(0);
	caller_.ttype!=-1?get_cdr().set(F_CLS_CALLER_TTYPE, caller_.ttype):void(0);
	caller_.sserv_type!=-1?get_cdr().set(F_CLS_CALLER_SSERV, caller_.sserv_type):void(0);
	caller_.sserv_no!=-1?get_cdr().set(F_CLS_CALLER_SSERV_NO, caller_.sserv_no):void(0);
	caller_.prefix!=-1?get_cdr().set(F_CLS_CALLER_PREFIX, caller_.prefix):void(0);
	caller_.ip_type!=-1?get_cdr().set(F_CLS_CALLER_IP_TYPE, caller_.ip_type):void(0);
	caller_.in_type!=-1?get_cdr().set(F_CLS_CALLER_IN_TYPE, caller_.in_type):void(0);
	caller_.trunk_carr!=-1?get_cdr().set(F_CLS_CALLER_TRUNK_CARR, caller_.trunk_carr):void(0);
	caller_.spcode!=-1?get_cdr().set(F_CLS_CALLER_SPCODE, caller_.spcode):void(0);
	caller_.sp_id!=-1?get_cdr().set(F_CLS_CALLER_SP_ID, caller_.sp_id):void(0);
	caller_.internet!=-1?get_cdr().set(F_CLS_CALLER_INTERNET, caller_.internet):void(0);

	called_.carr!=-1?get_cdr().set(F_CLS_CALLED_CARR, called_.carr):void(0);
	called_.ac!=-1?get_cdr().set(F_CLS_CALLED_AC, called_.ac):void(0);
	called_.sect!=-1?get_cdr().set(F_CLS_CALLED_SECT, called_.sect):void(0);
	called_.ttype!=-1?get_cdr().set(F_CLS_CALLED_TTYPE, called_.ttype):void(0);
	called_.sserv_type!=-1?get_cdr().set(F_CLS_CALLED_SSERV, called_.sserv_type):void(0);
	called_.sserv_no!=-1?get_cdr().set(F_CLS_CALLED_SSERV_NO, called_.sserv_no):void(0);
	called_.prefix!=-1?get_cdr().set(F_CLS_CALLED_PREFIX, called_.prefix):void(0);
	called_.ip_type!=-1?get_cdr().set(F_CLS_CALLED_IP_TYPE, called_.ip_type):void(0);
	called_.in_type!=-1?get_cdr().set(F_CLS_CALLED_IN_TYPE, called_.in_type):void(0);
	called_.trunk_carr!=-1?get_cdr().set(F_CLS_CALLED_TRUNK_CARR, called_.trunk_carr):void(0);
	called_.spcode!=-1?get_cdr().set(F_CLS_CALLED_SPCODE, called_.spcode):void(0);
	called_.sp_id!=-1?get_cdr().set(F_CLS_CALLED_SP_ID, called_.sp_id):void(0);
	called_.internet!=-1?get_cdr().set(F_CLS_CALLED_INTERNET, called_.internet):void(0);

	area_type_!=-1?get_cdr().set(F_CLS_AREA_TYPE, area_type_):void(0);
	local_type_!=-1?get_cdr().set(F_CLS_LOCAL_TYPE, local_type_):void(0);
	toll_type_!=-1?get_cdr().set(F_CLS_TOLL_TYPE, toll_type_):void(0);
	inet_toll_type_!=-1?get_cdr().set(F_CLS_INET_TOLL_TYPE, inet_toll_type_):void(0);
	call_trans_!=-1?get_cdr().set(F_CLS_CALL_TRANS, call_trans_):void(0);
	call_orient_!=-1?get_cdr().set(F_CLS_CALL_ORIENT, call_orient_):void(0);
	sett_obj_flag_!=-1?get_cdr().set(F_SETT_OBJ_FLAG, sett_obj_flag_):void(0);
}

void classify_proc::store_to_cdr2() {
	sett_obj_!=-1?get_cdr().set(F_CLS_SETT_OBJ, sett_obj_):void(0);
	sett_obj_type_!=-1?get_cdr().set(F_CLS_SETT_OBJ_TYPE, sett_obj_type_):void(0);
	sett_clsid_!=-1?get_cdr().set(F_CLS_SETT_CLSID, sett_clsid_):void(0);
	feerate_id_!=-1?get_cdr().set(F_CLS_FEERATE_ID, feerate_id_):void(0);
	sett_hop_!=-1?get_cdr().set(F_CLS_SETT_HOP, sett_hop_):void(0);
	fee_!=-1?get_cdr().set(F_CLS_FEE, fee_):void(0);
	currency_!=-1?get_cdr().set(F_CLS_CURRENCY, currency_):void(0);
	debit_!=-1?get_cdr().set(F_CLS_DEBIT, debit_):void(0);
	sett_period_!=""?get_cdr().set(F_CLS_SETT_PERIOD, sett_period_):void(0);
}
/*话单所属帐期*/ 
void classify_proc::parse_sett_period() {
	sett_period_ = get_cdr().get<string>(F_CLS_SETT_PERIOD);
	if( sett_period_=="" ) {
		sett_period_ = SETT_PERIOD_INS->parse_sett_period(get_cdr());
	}
}

/*结算金额*/ 
void classify_proc::compu_fee() {
	int total_deduct = data_finder::get_sett_classify(sett_clsid_)->deduct;
	fee_ = get_group_fee(feerate_id_)*total_deduct/100;
}

long long classify_proc::get_group_fee(int id) {
	typedef std::map<int, group_feerate_info, greater<int> > fee_t;
	int type = data_finder::get_group_feerate_type(id);
	if( type == 0 ) { // 单一费率
		const fee_t* info= data_finder::get_group_feerate(id);
		if( info == 0 ) {
			return 0;
		}
		BOOST_FOREACH(fee_t::const_reference i, *info) {
			return get_singal_fee(
								 data_finder::get_feerate(
														 i.second.fid
														 )
								 );
		}
		return 0;
	}
	// 费率组合
	if( type==1 || type==3 ) {	 // 1-时长组合 2-时间段组合 3-金额组合
		int value = 0;
		if( type==1 ) {
			value = get_cdr().get<int>(F_STD_DURATION);
		} else {
			value = get_cdr().get<int>(F_RAW_FEE);	// 已批价费用
		}
		typedef std::map<int, group_feerate_info, greater<int> > fee_t;
		long long total_fee = 0;
		const fee_t* info= data_finder::get_group_feerate(id);
		BOOST_FOREACH(fee_t::const_reference i, *info) {
			int low = i.second.lower_bound;
			int up = i.second.upper_bound;
			if( low == -1 ) {
				low = 0;
			}
			if( up == -1 ) {
				up = numeric_limits<int>::max();
			}
			if( value > low && value<=up ) {
				if( i.second.group_type==0 ) {
					total_fee += get_singal_group_fee(
													 data_finder::get_feerate(
																			 i.second.fid
																			 ),
													 value-low
													 );
					value = low;
				} else if( i.second.group_type==1 ) {
					total_fee += get_singal_group_fee(
													 data_finder::get_feerate(
																			 i.second.fid
																			 ),
													 value
													 );
					break;
				} else {
//  				total_fee += get_singal_group_fee(
//  												 data_finder::get_feerate(
//  																		 i.second.fid
//  																		 ),
//  												 value
//  												 );
				}
			}
		}
		return total_fee;
	} else {
		return get_ts_fee_total(id);
	}
	return 0;
}

long long classify_proc::get_singal_fee(const feerate_info* feerate) {
	if( feerate == 0 ) {
		return 0;
	}
	currency_ = feerate->currency;
	int duration = get_cdr().get<int>(F_STD_DURATION);
	long long value = 0;
	switch( feerate->unit ) {
	case 0:	// 跳次
		sett_hop_ = ((duration-1)/feerate->n_unit+1)*feerate->hop;
		value = ((sett_hop_-1)/feerate->n_unit+1)*feerate->feerate;
		break;
	case 1:	// 秒
		sett_hop_ = ((duration-1)/feerate->n_unit+1)*feerate->hop;
		value = ((duration-1)/feerate->n_unit+1)*feerate->feerate;
		break;
	case 2:	// 次
		value = feerate->feerate;
		break;
	case 3:	// 字节
	case 4:	// 金额
		break;
	}
	if( feerate->max_limit!=-1 && value > feerate->max_limit ) {
		value = feerate->max_limit;
	}
	return value;
}

long long classify_proc::get_singal_group_fee(const feerate_info* feerate, int value) {
	if( feerate == 0 ) {
		return 0;
	}
	currency_ = feerate->currency;
	sett_hop_ += ((value-1)/feerate->n_unit+1)*feerate->hop;
	long long fee = ((value-1)/feerate->n_unit+1)*feerate->feerate;
	if( feerate->max_limit!=-1 && fee > feerate->max_limit ) {
		fee = feerate->max_limit;
	}
	return fee;
}

long long classify_proc::get_ts_fee_total(int id) {
	typedef std::map<int, group_feerate_info, greater<int> > fee_t;
	long long total_fee = 0;
	int value = get_cdr().get<int>(F_STD_DURATION);
	wuya::datetime date = get_cdr().get<wuya::datetime>(F_STD_BEGIN_DATETIME);
	int caller_prov = data_finder::get_prov_from_area_code(caller_.ac);
	int called_prov = data_finder::get_prov_from_area_code(called_.ac);

	if( caller_prov == 26 || caller_prov == 31 || called_prov == 26 || called_prov==31 ) {
		date = date+wuya::timespan(0, 2);
	}

	const fee_t* info= data_finder::get_group_feerate(id);
	BOOST_FOREACH(fee_t::const_reference i, *info) {
		int low = i.second.lower_bound;
		int up = i.second.upper_bound;
		if( low == -1 ) {
			low = 0;
		}
		if( up == -1 ) {
			up = 24;
		}
		total_fee += get_singal_group_fee(
										 data_finder::get_feerate(
																 i.second.fid
																 ),
										 get_ts_duration(date, value, low, up)
										 );
	}
	return total_fee;
}

int classify_proc::get_ts_duration(const wuya::datetime& begin, int duration, int hour, int hour2) {
	static const int sec_per_day = 24*60*60;
	int b_time = begin.hour()*60*60+begin.minute()*60+begin.second();
	int e_time = b_time+duration;
	int day = 0;
	int b_sec = 0;
	int e_sec = 0; 
	int ret = 0;
	while( true ) {
		b_sec = day*sec_per_day+hour*60*60;
		e_sec = day*sec_per_day+hour2*60*60;
		if( b_time>=b_sec && b_time<e_sec ) {
			if( e_time<=e_sec ) {
				ret += duration;
				return ret;
			} else {
				ret += e_sec-b_time;
			}
		} else if( e_time>=b_sec && e_time<e_sec ) {
			ret += e_time-b_sec;
			return ret;
		} else if( e_time<b_sec || b_time>b_sec ) {
			break;
		} else {
			ret += (e_sec-b_sec);
		}
		++day;
	}
	return ret;
}


