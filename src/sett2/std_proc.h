#ifndef __STD_PROC_H__
#define __STD_PROC_H__

#include "proc_base.h"
#include "data_cache.h"

class std_proc : public proc_base {
public:
	~std_proc();
protected:
	
	virtual bool proc_record(cdr_ex& cdr, proc_context& ctx);
private:
		int FormatCallerTelNum(const char *TelNum, const  int priority, const  int operateType);
		int FormatCalledTelNum(const char *TelNum, const  int priority, const  int operateType, const char calledNature);
		int FormatThirdTelNum(const char *TelNum, const  int priority, const  int operateType);
		int FormatDateTime_Internet(cdr_ex& cdr);
		int FormatDateTime_1240_74(cdr_ex& cdr);
		int FormatDateTime_1240_B2(cdr_ex& cdr);
		int FormatDuration_1240(cdr_ex& cdr);

		int NetNumJump( const char *TelNum,  int *len );
		int CallingJump(const char *TelNum, const telno_rule *caller_rule,  int *len );

		int CalledJump( const char *TelNum, const telno_rule *called_rule,  int *len );
		int ChinaJump(const char *TelNum,  int *len );
		int MobileNumJump( const char *TelNum,  int *len );
		int SpecialJump(const char * TelNum, int *len);
		int SpecialJump2CalledNo(const  char * TelNum, int *len);
		int DistanceNumJump(const  char *TelNum,  int *len );
		void CalledFree( const  int operateType );
		void CallingFree( const   int operateType );
		telno format_number_;

		static const int STEP_CALLING_CHINA=1;	// ���Ҵ���: 86
		static const int STEP_CALLING_NET=3;	// ����
		static const int STEP_CALLING_NATION=5;	// ���Ҵ���
		static const int STEP_CALLING_HKM=7;	// ����ֻ�
		static const int STEP_CALLING_MOBILE=9	;// �ֻ�
		static const int STEP_CALLING_DISTANCE=13;	// ��������
		static const int STEP_CALLING_SPECIAL=15 ; 	//6L:EBk
		static const int STEP_CALLING_LOCAL=17;	// ���غ���

		static const int STEP_CALLED_NET=1;	// ����
		static const int STEP_CALLED_CHINA=5;	// ���Ҵ���: 86
		static const int STEP_CALLED_NATION=9	;// ���Ҵ���: ����
		static const int STEP_CALLED_MOBILE=13	;// �ֻ�
		static const int STEP_CALLED_SPECIAL = 19 ; //6L:EBk
		static const int STEP_CALLED_DISTANCE=17;	// ��������
		static const int STEP_CALLED_LOCAL=21;	// ���غ���
};
#endif // __STD_PROC_H__
