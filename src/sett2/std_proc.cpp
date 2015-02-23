#include "std_proc.h"

#include <stdio.h>
#include <stdlib.h>
#include "runtime_class.h"
#include "data_cache.h"
#include "data_finder.h"
#include <string>
#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <wuya/ace_log.h>
#include <my_datetime.h>
#include "log.h"

using namespace std;
using namespace wuya;
using namespace boost;

bool std_proc::proc_record(cdr_ex& cdr, proc_context& ctx) {
	string& fileType = ctx.get<string>(F_FILETYPE);
	if(fileType=="NET2NET")
	{
		
		string& caller_no = cdr.get<string>(F_CALLERNO);
		memset(&format_number_,0,sizeof(format_number_));

	//	if(caller_no == "0017329658478")
	//	{
	//		cout << "caller_no" << endl;
	//	}
		FormatCallerTelNum( caller_no.c_str(),-1,0);
		cdr.set(F_STD_CALLERNO, format_number_);
	
		string& called_no = cdr.get<string>(F_CALLEDNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( called_no.c_str(),-1,0,0);
		cdr.set(F_STD_CALLEDNO, format_number_);
		
		FormatDateTime_Internet(cdr);
		
		cdr.set(F_STD_DURATION, atoi(cdr.get<string>(F_DURATION).c_str()));
	}
	else if(fileType == "S1240V74")
	{
		string& caller_no = cdr.get<string>(F_CALLERNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCallerTelNum( caller_no.c_str(),-1,0);
		cdr.set(F_STD_CALLERNO, format_number_);
	
		string& called_no = cdr.get<string>(F_CALLEDNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( called_no.c_str(),-1,0,0);
		cdr.set(F_STD_CALLEDNO, format_number_);

		FormatDateTime_1240_74(cdr);
		FormatDuration_1240(cdr);
		
		
	}
	else if(fileType == "S1240CHB2")
	{
		string& caller_no = cdr.get<string>(F_CALLERNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCallerTelNum( caller_no.c_str(),-1,0);
		cdr.set(F_STD_CALLERNO, format_number_);
	
		string& called_no = cdr.get<string>(F_CALLEDNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( called_no.c_str(),-1,0,0);
		cdr.set(F_STD_CALLEDNO, format_number_);

		FormatDateTime_1240_B2(cdr);
		FormatDuration_1240(cdr);
		
	}
	else if(fileType == "201") 
	{
		string& caller_no = cdr.get<string>(F_CALLERNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCallerTelNum( caller_no.c_str(),-1,0);
		cdr.set(F_STD_CALLERNO, format_number_);
	
		string& called_no = cdr.get<string>(F_CALLEDNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( called_no.c_str(),-1,0,0);
		cdr.set(F_STD_CALLEDNO, format_number_);

		string& dest_no = cdr.get<string>(F_DESTNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( dest_no.c_str(),-1,0,0);
		cdr.set(F_STD_DESTNO, format_number_);

		string& begin_time  = cdr.get<string>(F_BEGIN_DATETIME);
	    cdr.set(F_STD_BEGIN_DATETIME, datetime(begin_time.c_str()));
		cdr.set(F_STD_DURATION, atoi(cdr.get<string>(F_DURATION).c_str()));
	}
	else if (fileType == "SMS")
	{
		//计费号码
		/*
 		string& charge_term_id = cdr.get<string>(F_CHARGE_TERM_ID);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCallerTelNum( charge_term_id.c_str(),-1,0);
		cdr.set(F_STD_CHARGE_TERM_ID, format_number_);
	   */
		//目的接收号码
		string& dest_term_id = cdr.get<string>(F_CALLEDNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( dest_term_id.c_str(),-1,0,0);
		cdr.set(F_STD_CALLEDNO, format_number_);
		
        //原始主叫号码
		string& src_term_id = cdr.get<string>(F_CALLERNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCallerTelNum( src_term_id.c_str(),-1,0);
		cdr.set(F_STD_CALLERNO, format_number_);     

		string& begin_time  = cdr.get<string>(F_BEGIN_DATETIME);
	    cdr.set(F_STD_BEGIN_DATETIME, datetime(begin_time.c_str()));
		cdr.set(F_STD_DURATION, 0);
	}
	else if( fileType =="NGN")
	{
		string& caller_no = cdr.get<string>(F_CALLERNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCallerTelNum( caller_no.c_str(),-1,0);
		cdr.set(F_STD_CALLERNO, format_number_);
	
		string& called_no = cdr.get<string>(F_CALLEDNO);
		memset(&format_number_,0,sizeof(format_number_));
		FormatCalledTelNum( called_no.c_str(),-1,0,0);
		cdr.set(F_STD_CALLEDNO, format_number_);
		
		string& begin_time  = cdr.get<string>(F_BEGIN_DATETIME);
		cdr.set(F_STD_BEGIN_DATETIME, datetime(begin_time.c_str()));
		
		string& end_time = cdr.get<string>(F_END_DATETIME);
		
		timespan ts = datetime(end_time.c_str()) - datetime(begin_time.c_str());
        
		cdr.set(F_STD_DURATION, (int)ts.get_total_seconds());
  		
	}
	else if( fileType =="IVR")
	{
		cout << "语音增值话单" << endl;
	}
	else
	{
		logerr << "错误的话单类型" << endl;
	}
    //cout << "call" << << format_number_.area_code
	return true;
}


std_proc::~std_proc() {

}

int std_proc::FormatCallerTelNum(const char *TelNum, const  int priority, const  int operateType) {
	
	 int    r;
	 int	len = 0;

	if( *TelNum == 0 )
	{
		telno_rule	caller_rule;
		caller_rule.operateType = STEP_CALLING_NATION;
		caller_rule.minlength = 0;
		caller_rule.maxlength = 0;
		return CallingJump( 0, &caller_rule, 0 );
	}
	std::vector<telno_rule> &caller_rule_vec = data_finder::get_caller_rule();
	
	for( size_t i = priority+1; i < caller_rule_vec.size(); i ++ )
	{
		if ((caller_rule_vec[i].operateType > operateType ) 
			&& ( strncmp(TelNum, caller_rule_vec[i].startChars, strlen( caller_rule_vec[i].startChars ) ) == 0 ))
		{
			r = CallingJump(TelNum, &(caller_rule_vec[i]), &len );
			if (r == 1 )
			{
				if ( FormatCallerTelNum( TelNum + len, caller_rule_vec[i].priority, caller_rule_vec[i].operateType ) == 0 )
					return 0;
				CallingFree( caller_rule_vec[i].operateType );	// 置空跳转所置的字段
			}
			else if( r == -1 )
			{
				continue;		//不符合跳号规则
			}
			else
				return r;

		}
	}
	return -1;

}
int std_proc::FormatCalledTelNum(const char *TelNum, const  int priority, const  int operateType, const char calledNature) {

	 int	 r;
	 int	len = 0;
	char	*p = NULL;
	
	std::vector<telno_rule> &called_rule_vec = data_finder::get_called_rule();

	for( size_t i = priority+1; i <called_rule_vec.size(); i ++ )
	{
             

			 if ( ( called_rule_vec[i].operateType > operateType )
				 && ( strncmp( TelNum, called_rule_vec[i].startChars, strlen( called_rule_vec[i].startChars ) ) == 0 ) )
			 {
				 len = 0;
				 if( ( r = CalledJump( TelNum, &(called_rule_vec[i]), &len ) ) == 1 )
				 {
					 if (FormatCalledTelNum( TelNum + len, called_rule_vec[i].priority, called_rule_vec[i].operateType, 0 ) == 0 )
					 {
						 return 0;
					 }
					 CalledFree( called_rule_vec[i].operateType );	// 置空跳转所置的字段
				 }
				 else if( r == -1 )
				 {
					 continue;
				 }
				 else
				 {			
					 return r;
				 }
		}
	}

	return -1;
}

int std_proc::FormatThirdTelNum(const char *TelNum, const  int priority, const  int operateType) {
	 int	 r;
	 int	len = 0;
	char	*p = NULL;
	std::vector<telno_rule> &called_rule_vec = data_finder::get_called_rule();

	for( size_t i = priority+1; i < called_rule_vec.size(); i ++ )
	{
		if ((called_rule_vec[i].operateType > operateType ) 
			&& ( strncmp( TelNum, called_rule_vec[i].startChars, strlen( called_rule_vec[i].startChars ) ) == 0 ) )
		{
			len = 0;
			if( ( r = CalledJump( TelNum, &(called_rule_vec[i]), &len ) ) == 1 )
			{
				if( FormatCalledTelNum( TelNum + len, called_rule_vec[i].priority, called_rule_vec[i].operateType, 0 ) == 0 )
				{
					return 0;
				}
				CalledFree( called_rule_vec[i].operateType );	// 置空跳转所置的字段
			}
			else if( r == -1 )
			{
				continue;
			}
			else
			{			
				return r;
			}
		}
	}
	return -1;
}

//=============================================================================
// 电话号码的网号跳转处理 
//-----------------------------------------------------------------------------
int std_proc::NetNumJump( const char* TelNum,  int *len )
{		
	int iRes=-1;
	const char* netcode = data_finder::get_cicno(TelNum);
	if(netcode != NULL)
	{
//  	if(strncmp(netcode,"0",1) != 0)
		{
			strcpy( format_number_.net_code, netcode);
			*len = strlen(netcode);
		}
		if( strlen(TelNum) == *len )
			iRes = 0;
		// 处理完成之后网号之后出现“0” 或者 “00”，或者是空
		else if (strcmp(TelNum + *len ,"0") == 0  || strcmp(TelNum + *len,"00") == 0 )
			iRes = 0;
		else
			iRes = 1;
	}
    return iRes;
	
}

//=============================================================================
// 主叫电话号码跳转处理 
//-----------------------------------------------------------------------------
int std_proc::CallingJump( const char *TelNum, const telno_rule *caller_rule, int *len )
{


	if( ( strlen( TelNum ) < (size_t)caller_rule->minlength ) || ( strlen( TelNum ) > (size_t)caller_rule->maxlength ) )
		return -1;
		
	switch( caller_rule->operateType )
	{
		case STEP_CALLING_NET:
			return NetNumJump( TelNum, len );
			
		case STEP_CALLING_CHINA:
			return ChinaJump( TelNum, len );

		case STEP_CALLING_NATION:
			//strcpy( format_number_.CountryNum, "000" );
			strcpy( format_number_.area_code, "00" );
			return 0;
			
		case STEP_CALLING_MOBILE:
			return MobileNumJump( TelNum, len );
							
		case STEP_CALLING_DISTANCE:
			return DistanceNumJump( TelNum, len );
			
	    case STEP_CALLING_SPECIAL:
	        	return SpecialJump( TelNum, len );
	
		case STEP_CALLING_LOCAL:
			if(strlen(TelNum) >= 5 && (strncmp(TelNum,"193",3) == 0 || strncmp(TelNum,"179",3) == 0)) /*如果本地号码中带有网号,取出网号,并将剩下的号码返回 */
			{
				 int	temp;
				if(NetNumJump(TelNum,&temp) != -1)
				{
					return 0;
				}
			}
			strcpy( format_number_.user_code, TelNum );
			return 0;

		case STEP_CALLING_HKM:

			return DistanceNumJump( TelNum, len );

			return 0;
			
		default:	
			return -1;
	}
}

//=============================================================================
// 被叫电话号码跳转处理 
//-----------------------------------------------------------------------------
int std_proc::CalledJump(const char *TelNum, const telno_rule *called_rule,  int *len )
{	
	if( ( strlen( TelNum ) < (size_t)called_rule->minlength ) || ( strlen( TelNum ) > (size_t)called_rule->maxlength ) )
		return -1;
		
	switch( called_rule->operateType ){
		case STEP_CALLED_NET:
			return NetNumJump( TelNum, len );
			
		case STEP_CALLED_CHINA:
			return ChinaJump( TelNum, len );
			
		case STEP_CALLED_NATION:
			{
			if( strncmp( TelNum, "000", 3 ) == 0 )	//这样的号码非法
				return 0;
			if( strncmp( TelNum, "00", 2 ) == 0 )
				*len = 2;
			const int countryLenMax=5;
		    char countryTmp[10];
			for(int countryLen = countryLenMax; countryLen>0; countryLen--)
			{
				memset(countryTmp,0,sizeof(countryTmp));
				memcpy( countryTmp, TelNum + *len, countryLen );
				countryTmp[countryLen]='\0';
				if(data_finder::is_valid_country_id(countryTmp))
				{
					//sprintf( format_number_.area_code, "00%s",countryTmp );
					sprintf( format_number_.area_code, "0%s",countryTmp );
					strcpy( format_number_.user_code, TelNum + *len + countryLen );
					return 0;
				}
			}
			// 如果网号存在，国家号码不存在，返回网号
			if (strlen(format_number_.net_code )>0)
				return 0;		
			else
				return -1;
			return -1;
			}
		case STEP_CALLED_MOBILE:
		
			return MobileNumJump( TelNum, len );
				
		case STEP_CALLED_DISTANCE:
	
			return DistanceNumJump( TelNum, len );
		case STEP_CALLED_SPECIAL:
	
		    	return SpecialJump2CalledNo( TelNum, len );
		case STEP_CALLED_LOCAL:
			if(strlen(TelNum) >= 5 && (strncmp(TelNum,"193",3) == 0 || strncmp(TelNum,"179",3) == 0)) /*如果本地号码中带有网号,取出网号,并将剩下的号码返回 */
			{
				 int	temp;
				if(NetNumJump(TelNum,&temp) != -1)
				{
					return 0;
				}
			}

			strcpy( format_number_.user_code, TelNum );
			return 0;

		default:	
			return -1;
	}
}
//=============================================================================
// 电话号码的中国(86、0086)区号跳转处理 
//-----------------------------------------------------------------------------
int std_proc::ChinaJump( const char *TelNum,  int *len )
{
	if( strncmp( TelNum, "86", 2 ) == 0 )
		*len = 2;
	else if( strncmp( TelNum, "0086", 4 ) == 0 )
		*len = 4;	
	if( strlen( TelNum ) > (size_t)*len ){
		strcpy( format_number_.country_code, "0086" );
		return 1;
	}
	return -1;
}
//=============================================================================
// 电话号码的移动号码和区号跳转处理 
//-----------------------------------------------------------------------------
int std_proc::MobileNumJump( const char *TelNum,  int *len )
{

	string  Mobnum;
	string	TelNo;
	TelNo = TelNum;
	
	if( *TelNum == '0' )	//本地号码
		*len = 1;
	else			//外地号码
		*len = 0;
	strcpy( format_number_.user_code, TelNum + *len );
    
   for (int i=7;i<9;++i)
	{//分两次查找，第一次假设3H码为4位，第二次假设3H码为5位
	    Mobnum=TelNo.substr(*len,i);
		const hcode_info* area_code = data_finder::get_mobile_info(Mobnum.c_str());
		if(area_code != NULL)
		{
			if(area_code->dis_no=="0")
			{
				strcpy(format_number_.area_code,area_code->dis_no.c_str());
				break;
			}
		}
   }

	return 0;
}


//=============================================================================
// 
//-----------------------------------------------------------------------------

int std_proc::SpecialJump(const  char * TelNum, int *len)
{

//    int	i;
	char	tmp[4];
	if(format_number_.area_code[0] != 0)
	{
		*len = 0;
		return 1;
	}

	memset(tmp,0,sizeof(tmp));
	
	if( *TelNum == '0' )
		*len = 1;
	else
		*len = 0;
		
	if( *( TelNum + *len ) == '1' || *( TelNum + *len ) == '2' ){
		memcpy( tmp, TelNum + *len, 2 );
		tmp[2] = 0;
		*len += 2;
	}
	else{
		memcpy( tmp, TelNum + *len, 3 );
		tmp[3] = 0;
		*len += 3;
	}
	if(data_finder::is_valid_area_code(tmp))
	{
		if(data_finder::is_special_number(TelNum + *len))
		{
              sprintf(format_number_.area_code,"0%s",tmp);
			  strcpy(format_number_.user_code,TelNum+ *len);
			  return 0;
		}  
		*len = 0;		
	     return 1;
	}
   	 return -1;
}

int std_proc::SpecialJump2CalledNo(const  char * TelNum, int *len)
{

	//    int	i;
	char	tmp[4];
	if(format_number_.area_code[0] != 0)
	{
		*len = 0;
		return 1;
	}

	memset(tmp,0,sizeof(tmp));

	if( *TelNum == '0' )
		*len = 1;
	else
		*len = 0;

	if( *( TelNum + *len ) == '1' || *( TelNum + *len ) == '2' ){
		memcpy( tmp, TelNum + *len, 2 );
		tmp[2] = 0;
		*len += 2;
	}
	else{
		memcpy( tmp, TelNum + *len, 3 );
		tmp[3] = 0;
		*len += 3;
	}
	//if(data_finder::is_valid_area_code(tmp))
	//{
	if(strcmp(tmp,"22")==0)
	{
		if(data_finder::is_special_number(TelNum + *len))
		{
			sprintf(format_number_.area_code,"0%s",tmp);
			strcpy(format_number_.user_code,TelNum+ *len);
			return 0;
		}  
		*len = 0;		
		return 1;
	}
	return -1;
}

//=============================================================================
// 电话号码的长途区号跳转处理 
//-----------------------------------------------------------------------------
int std_proc::DistanceNumJump( const char *TelNum,  int *len )
{

//	int	i;
	char	tmp[4];
	
	if( *TelNum == '0' )
		*len = 1;
	else
		*len = 0;
		
	if( *( TelNum + *len ) == '1' || *( TelNum + *len ) == '2' ){
		memcpy( tmp, TelNum + *len, 2 );
		tmp[2] = 0;
		*len += 2;
	}
	else{
		memcpy( tmp, TelNum + *len, 3 );
		tmp[3] = 0;
		*len += 3;
	}

	//对于以20开头的号码。判断是200卡、201卡或者是区号 ＋ 本地号码 

	if(strncmp(TelNum,"20",2) == 0)
	{
		
		switch(*(TelNum + *len))
		{
			case '0':  //如果是200＋其他号码，认为是200卡号码 
				sprintf(format_number_.user_code,TelNum);				
				return 0;
			case '1':  //如果是201 ＋ 其他号码 
				
				if( (*(TelNum + *len + 1)) == '3') //如果类似于2013 + 其他号码 
				{	
					if(strlen(TelNum) == 13)
					{
						//strcpy( m_sFormatNumTmp.DistanceNum, "020" );
						//strcpy( format_number_.area_code, "020" );
						strcpy( format_number_.area_code, "20" );
						strcpy( format_number_.user_code,TelNum + *len);
						return 0;
					}
				}
				
				sprintf(format_number_.user_code,TelNum);				
				
				return 0;
			default:   //其他 

				break;
		}
	}

	//其他处理  
	
	if( data_finder::is_valid_area_code(tmp))
	{
		//sprintf( m_sFormatNumTmp.DistanceNum, "0%s", tmp );
		//sprintf( format_number_.area_code, "0%s",tmp );
		sprintf( format_number_.area_code, "%s",tmp );
		
		if( strlen( TelNum + *len ) > 8 ){
			if( !strncmp( TelNum + *len, tmp, strlen( tmp ) ) )
			{
				*len += strlen( tmp );
			}
			else if( *( TelNum + *len ) == '0' && !strncmp( TelNum + *len, format_number_.area_code, strlen( format_number_.area_code) ) )
			{
				*len += strlen( format_number_.area_code );
			}
			else if (strlen(format_number_.net_code )>0)			//如果是接入号码＋区号＋0＋13开头的移动号码
			{
				if (strncmp(TelNum + strlen( format_number_.area_code ),"013",3) == 0)
					*len+=1;
				else if (strncmp(TelNum + strlen( format_number_.area_code ),"13",2) == 0)
					*len = *len;
				else if (strncmp(TelNum + strlen( format_number_.area_code ),"0",1) == 0)
				{
					return 1;			//号码不正确，并且号码为 179xx+0XXX+0XXXX....
				}
			}
					
		}
		else if (( strlen( TelNum + *len ) > 5 ) && ( strlen( TelNum + *len ) <= 8 ))	//双区号加特服号码
		{

			if( !strncmp( TelNum + *len, tmp, strlen( tmp ) ) )
			{		
				if(data_finder::is_special_number(TelNum))
				   {
					   *len = *len +1;			//把第一个区号后面的0待上
						memset(format_number_.area_code,0,sizeof(format_number_.area_code));
				   }
                  
			}
			else if( *( TelNum + *len ) == '0' && !strncmp( TelNum + *len, format_number_.area_code, strlen( format_number_.area_code ) ) )
			{
		
				if(data_finder::is_special_number(TelNum))
				   {
					   *len = *len +1;			//把第一个区号后面的0待上
						memset(format_number_.area_code,0,sizeof(format_number_.area_code));
				   }
				
			}
		}		
		else if (strlen(TelNum + *len)==0 )	//如果电话号码只有区号，正常返回
		{
			return 0;
		}
		else if (strlen(format_number_.net_code )>0 && strlen(TelNum+*len)<3)	//179XX0XX－xx,
		{
			return 0;
		}		
		
		return 1;
	}
	//只有区号的
	else if (strlen(format_number_.net_code )>0)
	{
		*len = 0;
		return 1;		//正常结束，只写入m_sFormatNumTmp.NetNum
	}
	
	return -1;
}



void std_proc::CallingFree( const  int operateType  )
{

}

void std_proc::CalledFree( const  int operateType  )
{

}

int std_proc::FormatDateTime_Internet(cdr_ex& cdr)
{
	string& start_year=cdr.get<string>(F_BEGIN_YEAR);
	string& start_month=cdr.get<string>(F_BEGIN_MONTH);
	string& start_day=cdr.get<string>(F_BEGIN_DAY);
	string& start_hour=cdr.get<string>(F_BEGIN_HOUR);
	string& start_minute=cdr.get<string>(F_BEGIN_MINUTE);
	string& start_second=cdr.get<string>(F_BEGIN_SECOND);

	char  start_date[15];
	//char  end_date[15];
	memset(start_date,0,sizeof(start_date));
	//memset(end_date,0,sizeof(end_date));

	if(start_year.length()==1)
	{
		sprintf(start_date,"%03.3d", datetime::current_time().year());
		sprintf(start_date+3,"%01.1s",start_year.c_str());

    }
	else
	{
		sprintf(start_date,"%02.2d",datetime::current_time().year());
		sprintf(start_date+2,"%01.1s",start_year.c_str());

	}
    sprintf(start_date,"%04.4s%02.2s%02.2s%02.2s%02.2s%02.2s", start_date,start_month.c_str()
		,start_day.c_str(),start_hour.c_str(),start_minute.c_str(),
		start_second.c_str());
	/*
	string* end_year=cdr.get_value<string>("end_year");
	string* end_month=cdr.get_value<string>("end_month");
	string* end_day=cdr.get_value<string>("end_day");
	string* end_hour=cdr.get_value<string>("end_hour");
	string* end_minute=cdr.get_value<string>("end_minute");
	string* end_second=cdr.get_value<string>("end_second");
	
	memset(year,0,sizeof(year));
    sprintf(year,"%04d",dt.year());
	if((*end_year).length()==1)
	{
		year[3] = '\0';
		memcpy(end_date, year,3);
		memcpy(end_date+3,(*start_year).c_str(),(*end_year).length());

    }
	else
	{
		year[2] = '\0';
		memcpy(end_date, year,2);
		memcpy(end_date+2,(*start_year).c_str(),(*end_year).length());

	}


	sprintf(end_date,"%04d%02d%02d%02d%02d%02d",lexical_cast<int>(end_date),lexical_cast<int>(*end_year),lexical_cast<int>(*end_month),
		lexical_cast<int>(*end_day),lexical_cast<int>(*end_hour),lexical_cast<int>(*end_minute),lexical_cast<int>(*end_second));
	
    */
	cdr.set(F_STD_BEGIN_DATETIME, datetime(start_date));
	//cdr.set(F_STD_END_DATETIME , datetime(end_date));
	return 0;

}

int std_proc::FormatDateTime_1240_74(cdr_ex& cdr)
{
	string& answer_datetime=cdr.get<string>(F_ANSWER_DATETIME);
	char std_answer_datetime[15];
	memset(std_answer_datetime,0,sizeof(std_answer_datetime));
   	sprintf(std_answer_datetime,"%02.2d",datetime::current_time().year());
	sprintf(std_answer_datetime+2,"%012.12s",answer_datetime.c_str());

	cdr.set(F_STD_BEGIN_DATETIME, datetime(std_answer_datetime));

	return 0;

}

int std_proc::FormatDateTime_1240_B2(cdr_ex& cdr)
{
	string& answer_datetime=cdr.get<string>(F_ANSWER_DATETIME);
	char std_answer_datetime[15];
	memset(std_answer_datetime,0,sizeof(std_answer_datetime));
 	sprintf(std_answer_datetime,"%014.14s",answer_datetime.c_str());

	cdr.set(F_STD_BEGIN_DATETIME, datetime(std_answer_datetime));

	return 0;

}

int std_proc::FormatDuration_1240(cdr_ex& cdr)
{
	// HHHMMSST->HHH_MM_SS_
	const char* p = cdr.get<string>(F_DURATION).c_str();
	char buf[10];
	memset(buf, 0, 10);
	memcpy(buf, p, 3);
	memcpy(buf+4, p+3, 2);
	memcpy(buf+7, p+5, 2);
    timespan ts(0,atoi(buf),atoi(buf+4),atoi(buf+7));
	int std_duration = ts.get_total_seconds();
	cdr.set(F_STD_DURATION, std_duration);
	return 0;
}

