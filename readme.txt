<<<��������<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

��������Ҫ���µĿ���б��룬���б��붼�þ�̬�⣬������ʱ���������ٵ���С��
1. ACE 5.6
2. boost 1.34.1
3. otl
4. instantclient��oracle sdk with oci support
5. wuya

Ϊ�˽���ͬ�����µı���Ķ�������С�����п��������Ŀ¼��ʽ��ţ�
[LIBS_ROOT]
����ACE_wrappers
��  ����ace
��  ����include
��  ��  ����makeinclude
��  ����lib
����boost
��  ����include
��  ��  ����boost
��  ��      ����algorithm
��  ��      ����archive
��  ��      ����......
��  ��      ����xpressive
��  ����lib
����instantclient
��  ����include
��  ����lib
����otl
����wuya
    ����include
        ����wuya

<<<VC2005����<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
1. �ֱ������������includeĿ¼��lib��Ŀ¼��
2. �򿪹��̣�src\sett2_vc8.sln
3. build����
4. ִ��

<<<Unix/Linux����<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
1. ����LIBS_ROOT��������
2. cd src
   make // ��������
   make clean // ����
   make rebuild // ���±���
   make CC=xxxx // ʹ���������������б��룬��aCC��xlC
   make release // ���뷢�а�
   make backup // �������ļ�������bakĿ¼��
3. cd bin
   sett2

<<<����ʱ<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

����ʱ����Ҫoracle client����instant oracle client֧�֡�


wuya
