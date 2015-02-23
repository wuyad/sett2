#ifndef __FTP_CLIENT_H__
#define __FTP_CLIENT_H__

#include <set>
#include <vector>
#include <queue>
#include <string>
#include <sstream>
#include <boost/scoped_ptr.hpp>
#include <ace/SOCK_Stream.h>
#include <ace/SOCK_Connector.h>
#include <ace/Time_Value.h>
#include <ace/OS_NS_sys_time.h>
#include <ace/OS_NS_unistd.h>
#include <ace/SOCK_Acceptor.h>
#include <my_datetime.h>

namespace wuya {
	class notification;
	class trans_notification;
	class ftp_reply;
	class observer_set : public std::set<notification*> {
	public:
		typedef std::set<notification*> base_t;
		bool attach(notification* p);
		bool detach(notification* p);
	};
	struct logon_info {
		logon_info();
		logon_info(const std::string& hostname, unsigned short port=21, const std::string& username="anonymous", 
				   const std::string& password="anonymous@user.com", const std::string& account="");
		std::string        hostname;	///< name or ip-address of the ftp-server
		unsigned short     port;		///< port of the ftp-server
		std::string        username;	///< username for ftp-server
		std::string        password;	///< password for ftp-server
		std::string        account;		///< account mostly needed on ftp-servers running on unix/linux
	};
	enum TRANSFER_MODE {
		TM_STREAM, TM_BLOCK, TM_COMPRESSED
	};
	enum FILE_STRUCTURE {
		FS_FILE, FS_RECORD, FS_PAGE
	};
	enum DATA_CHANNEL_CMD {
		CMD_LIST, CMD_NLST, CMD_RETR, CMD_STOR, CMD_STOU, CMD_APPE
	};   
	enum TRANS_TYPE {
		TT_ASCII, TT_EBCDIC, TT_IMAGE,TT_LOCALBYTE
	};
	enum FORMAT_TYPE {
		FT_NONPRINT, FT_TELNET_FORMAT, FT_CARRIAGE_CONTROL
	};
	struct ftp_file_stat {
		std::string filename;
		std::string pathname;
		std::string linkname;
		wuya::datetime mtime;
		long file_size;
		std::string attr;
		std::string uid;
		std::string gid;
		bool is_dir;
		bool is_link;

		ftp_file_stat();
	};
	class ftp_client {
	public:
		ftp_client(unsigned int timeout=10, unsigned int response_wait=0);
		virtual ~ftp_client();
	public:
		void attach_observer(notification* observer);
		void detach_observer(notification* observer);

		bool is_connected() const;
		bool is_transferring_data() const;
		void set_resume_mode(bool enable=true);

		bool login(const logon_info& info);
		bool login(const std::string& hostname, unsigned short port=21, const std::string& username="anonymous", 
				   const std::string& password="anonymous@user.com", const std::string& account="");
		bool logout();
		const logon_info& last_logon_info() const;

		bool rm(const std::string& filename) const;
		bool rename(const std::string& old_name, const std::string& new_name) const;
		bool rmdir(const std::string& dir) const;
		bool mkdir(const std::string& dir) const;
		bool pwd() const;
		bool cdup() const;
		bool cd(const std::string& dir) const;

		bool abort() const;
		bool system() const;
		bool noop() const;
		bool set_type(TRANS_TYPE type, FORMAT_TYPE ft, long size=0) const;
		bool file_structure(FILE_STRUCTURE fs) const;
		bool transfer_mode(TRANSFER_MODE mode) const;
		bool site(const std::string& cmd) const;
		bool status(const std::string& path) const;
		bool status() const;
		bool help() const;
		bool help(const std::string& topic) const;

		bool reinitialize() const;
		bool restart(int pos) const;
		bool file_size(const std::string& filename, long& size) const;
		bool file_modify_time(const std::string& filename, std::string& mod_time) const;
	public:
		bool port(const std::string& ip, unsigned short int port) const;
		bool passive(ACE_INET_Addr& addr) const;
		bool ll(const std::string& path, std::vector<std::string>& file_list, bool is_pasv=false) const;
		bool ls(const std::string& path, std::vector<std::string>& file_list, bool is_pasv=false) const;
		bool ll(const std::string& path, std::vector<ftp_file_stat>& file_list, bool is_pasv=false) const;
		bool download_file(const std::string& remote_filename, 
						   const std::string& local_filename, 
						   TRANS_TYPE type=TT_IMAGE, 
						   bool is_pasv=false) const;
		bool upload_file(const std::string& local_filename, 
						 const std::string& remote_filename, 
						 bool store_unique=false, 
						 TRANS_TYPE type=TT_IMAGE,
						 bool is_pasv=false) const;
	protected:
		bool execute_data_command(DATA_CHANNEL_CMD cmd, const std::string& path, 
								  TRANS_TYPE type, FORMAT_TYPE ftype, bool is_pasv,
								  long offset, trans_notification* observer) const;
		observer_set& get_observers();
	private:
		bool open_active_data_connection(ACE_SOCK_Stream& stream, DATA_CHANNEL_CMD cmd, 
										 const std::string& path, int offset) const;
		bool open_passive_data_connection(ACE_SOCK_Stream& stream, DATA_CHANNEL_CMD cmd,
										  const std::string& path, int offset) const;
		bool send_data(trans_notification* observer, ACE_SOCK_Stream& stream) const;
		bool receive_data(trans_notification* observer, ACE_SOCK_Stream& stream) const;
		bool transfer_data(DATA_CHANNEL_CMD cmd, trans_notification* observer, 
						   ACE_SOCK_Stream& stream) const;

		std::string get_cmd_str(DATA_CHANNEL_CMD cmd, const std::string& path) const;
		bool simple_error_check(const ftp_reply& reply) const;

		bool send_command(const std::string& cmd) const;
		bool send_command(const std::string& cmd, ftp_reply& reply) const;
		bool send_command_with_check(const std::string& cmd) const;
		/**
		 * This function gets the server response.
		 * A server response can exists of more than one line. This function
		 * returns the full response (multiline).
		 * 
		 * @param r
		 * 
		 * @return reply of the server to a command.
		 */
		bool get_response(ftp_reply& reply) const;
		bool get_single_response_line(std::string& reponse) const;

		bool open_control_channel(const std::string& hostname, unsigned short int port=21);
		void close_control_channel();
		void report_error(const std::string& error_msg, const std::string& filename, int line_num) const;
	private:
		mutable char buffer_[2048];	///< buffer for sending and receiving
		const unsigned int timeout_; ///< timeout for socket-functions
		const unsigned int response_wait_; ///< sleep time between receive calls to socket when getting the response
		boost::scoped_ptr<ACE_SOCK_Stream> socket_;///< socket for connection to ftp-server
		observer_set observers_; ///< list of observers, which are notified about particular actions
		bool resume_if_possible_; ///< try to resume download/upload if possible
		mutable bool trans_now_; ///< if true, a file transfer is in progress
		mutable bool abort_trans_;///< indicates that a running filetransfer should be canceled
		logon_info last_logon_info_; ///< logon-info, which was used at the last call of login
		mutable std::queue<std::string> reponse_queue; ///< buffer for server-responses
	};
}

//----------------------------------------------------------------------------------------------
namespace wuya {
	inline bool observer_set::attach(notification* p) {
		return insert(p).second;
	}
	inline bool observer_set::detach(notification* p) {
		return erase(p)==1;
	}
	inline logon_info::logon_info() {
	}
	inline logon_info::logon_info(const std::string& h, unsigned short p, const std::string& u, 
								  const std::string& pass, const std::string& a):hostname(h),
	port(p),username(u),password(pass),account(a) {
	}
	class ftp_reply {
		std::string reponse_;

		/// Holds the reply code.
		class reply_code {
			char scode_[4];
		public:
			reply_code() {
				std::fill_n(scode_, sizeof(scode_)/sizeof(char), 0);
			}
			const char* value() const {
				return scode_;
			}
			bool set(const std::string& scode) {
				if( scode.length()!=3 ||
					scode[0]<'1' || scode[0]>'5' ||
					scode[1]<'0' || scode[1]>'5' ) {
					std::fill_n(scode_, sizeof(scode_)/sizeof(char), 0);
					return false;
				}
				std::copy(scode.begin(), scode.end(), scode_);
				return true;
			}

			bool is_positive_reply() const {
				return is_positive_preliminary_reply() || is_positive_completion_reply() 
				|| is_positive_intermediate_reply();
			}
			bool is_negative_reply() const {
				return is_transient_negative_completion_reply() || is_permanent_negative_completion_reply(); 
			}

			bool is_positive_preliminary_reply() const {
				return scode_[0] == '1';
			}
			bool is_positive_completion_reply() const {
				return scode_[0] == '2';
			}
			bool is_positive_intermediate_reply() const {
				return scode_[0] == '3';
			}
			bool is_transient_negative_completion_reply() const {
				return scode_[0] == '4';
			}
			bool is_permanent_negative_completion_reply() const {
				return scode_[0] == '5';
			}

			bool is_reffering_to_syntax() const {
				return scode_[1] == '0';
			}
			bool is_reffering_to_information() const {
				return scode_[1] == '1';
			}
			bool is_reffering_to_connections() const {
				return scode_[1] == '2';
			}
			bool is_reffering_to_authentication_and_accounting() const {
				return scode_[1] == '3';
			}
			bool is_reffering_to_unspecified() const {
				return scode_[1] == '4';
			}
			bool is_reffering_to_file_system() const {
				return scode_[1] == '5';
			}
		} code_;
	public:
		bool set(const std::string& r) {
			reponse_ = r;
			if( reponse_.length()>2 )
				return code_.set(reponse_.substr(0, 3));
			return false;
		}
		const std::string& value() const {
			return reponse_;
		}
		const reply_code& code() const {
			return code_;
		}
	};

	class notification {
	public:
		virtual void on_internal_error(const std::string& error_msg, const std::string& filename,
									   int line_num) {
		}
		virtual void on_begin_receiving_data() {
		}
		virtual void on_end_receiving_data(long received_bytes) {
		}
		virtual void on_bytes_received(const char* buffer, long received_Bytes) {
		}
		virtual void on_bytes_sent(const char* buffer, long sent_bytes) {
		}
		virtual void on_pre_receive_file(const std::string& source_file, const std::string& target_file,
										 long file_size) {
		}
		virtual void on_pre_send_file(const std::string& source_file, const std::string& target_file,
									  long file_size) {
		}
		virtual void on_post_receive_file(const std::string& source_file, const std::string& targetFile,
										  long file_size) {
		}
		virtual void on_post_send_file(const std::string& source_file, const std::string& target_file,
									   long file_size) {
		}
		virtual void on_send_command(const std::string& command) {
		}
		virtual void on_response(const ftp_reply& reply) {
		}
	};
	class trans_notification {
	public:
		virtual void on_bytes_received(const char* buffer, long received_bytes) {
		}
		virtual void on_pre_bytes_send(char* buffer, size_t buffer_size, size_t& bytes_to_send) {
		}
	};

	class file_trans : public trans_notification {
		FILE* file_;
	public:
		enum ORIGIN {
			OR_BEGIN=SEEK_SET, OR_END=SEEK_END, OR_CURRENT=SEEK_CUR
		};

		file_trans() : file_(0) {
		}
		~file_trans() {
			close();
		}

		bool open(const char* filename, const char* mode) {
			file_ = fopen(filename, mode);
			return file_!=0;
		}

		bool close() {
			FILE* file = file_;
			file_ = NULL;
			return file && fclose(file)==0;
		}

		bool seek(long offset, ORIGIN org) {
			return file_ && fseek(file_, offset, org)==0;
		}

		long tell() {
			if( !file_ )
				return -1L;
			return ftell(file_);
		}

		size_t write(const char* buffer, size_t count) {
			if( !file_ )
				return 0;
			return fwrite(buffer, 1, count, file_);
		}

		size_t read(char* buffer, size_t count) {
			if( !file_ )
				return 0;
			return fread(buffer, 1, count, file_);
		}

		virtual void on_bytes_received(const char* buffer, long received_bytes) {
			write(buffer, received_bytes);
		}

		virtual void on_pre_bytes_send(char* buffer, size_t buffer_size, size_t& bytes_to_send) {
			bytes_to_send = read(buffer, buffer_size);
		}
	};

	class string_trans : public trans_notification {
		std::stringstream os;
	public:
		bool get_next_line(std::string& line) {
			getline(os, line);
			std::string::size_type len = line.size();
			if(len>0 && line[len-1]=='\r') {
				line[len-1] = '\0';
			}
			if( os.fail() ) {
				return false;
			}
			return true;
		}

		virtual void on_bytes_received(const char* buffer, long received_bytes) {
			os.write(buffer, received_bytes);
		}
	};
	class ftp_list_parse {
	public:
		bool parse(const std::string& line, ftp_file_stat& file_stat);
	private:
		bool try_parse_EPLF(const std::string& line, ftp_file_stat& file_stat);
		bool try_parse_unix_style(const std::string& line, ftp_file_stat& file_stat);
		bool try_parse_multi_net(const std::string& line, ftp_file_stat& file_stat);
		bool try_parse_msdos_style(const std::string& line, ftp_file_stat& file_stath);
		
		bool check_month(const char* buffer, const char* month_name) const;
		int  get_month(const char* buffer, int len) const;
		bool get_long(const char* str, int len, long& result) const;
	};
	inline ftp_client::ftp_client(unsigned int timeout, unsigned int response_wait):
	timeout_(timeout),response_wait_(response_wait),resume_if_possible_(false),
	trans_now_(false),abort_trans_(false) {
	}
	inline void ftp_client::attach_observer(notification* observer) {
		assert(observer);
		observers_.attach(observer);
	}
	inline void ftp_client::detach_observer(notification* observer) {
		observers_.detach(observer);
	}
	inline observer_set& ftp_client::get_observers() {
		return observers_;
	}
	inline void ftp_client::set_resume_mode(bool enable) {
		resume_if_possible_=enable;
	}
	inline bool ftp_client::is_connected() const {
		return(socket_ && socket_->get_handle () != ACE_INVALID_HANDLE);
	}
	inline bool ftp_client::is_transferring_data() const {
		return trans_now_;
	}
	inline bool ftp_client::login(const std::string& hostname, unsigned short port, const std::string& username, 
								  const std::string& password, const std::string& account) {
		logon_info info;
		info.hostname = hostname;
		info.port = port;
		info.username = username;
		info.password = password;
		info.account = account;
		return login(info);
	}

	inline bool ftp_client::login(const logon_info& info) {
		last_logon_info_ = info;

		enum {
			LO=-2,		///< Logged On
			ER=-1,		///< Error
		};

		int logon_seq[9] = {
			// this array stores all of the logon sequences for the various firewalls 
			// in blocks of 3 nums.
			// 1st num is command to send, 
			// 2nd num is next point in logon sequence array if 200 series response  
			//         is rec'd from server as the result of the command,
			// 3rd num is next point in logon sequence if 300 series rec'd
			0,LO,3,    1,LO, 6,   2,LO,ER // no firewall
		};

		if( is_connected() )
			logout();

		if( !open_control_channel(info.hostname, info.port) )
			return false;

		// get initial connect msg off server
		ftp_reply reply;
		if( !get_response(reply) || !reply.code().is_positive_completion_reply() )
			return false;

		int next_cmd_index=0;
		std::string cmd;
		// go through appropriate logon procedure
		while( true ) {
			switch( logon_seq[next_cmd_index] ) {
			case 0:
				cmd="USER " + info.username;
				break;
			case 1:
				cmd="PASS " + info.password;
				break;
			case 2:
				cmd="ACCT " + info.account;
				break;
			}

			// send command, get response
			if( !send_command(cmd, reply) )
				return false;

			if( !reply.code().is_positive_completion_reply() && !reply.code().is_positive_intermediate_reply() )
				return false;

			const unsigned int first_digit = atoi(reply.code().value())/100;
			next_cmd_index=logon_seq[next_cmd_index + first_digit-1]; //get next command from array
			switch( next_cmd_index ) {
			case ER: // ER means somewhat has gone wrong
				{
					report_error("Logon failed.", __FILE__, __LINE__);
				}
				return false;
			case LO: // LO means we're fully logged on
				return true;
			}
		}
		return false;
	}
	inline bool ftp_client::get_response(ftp_reply& reply) const {
		std::string res;
		if( !get_single_response_line(res) )
			return false;

		if( res.length()>3 && res.at(3)=='-' ) {
			const int retcode=atoi(res.c_str());
			std::string single_line(res);
			// handle multi-line server responses
			while( !(single_line.length()>3 && 
					 single_line.at(3)==' ' &&
					 atoi(single_line.c_str())==retcode) ) {
				if( !get_single_response_line(single_line) )
					return false;
				res += "\r\n" + single_line;
			}
		}
		bool ret = reply.set(res);

		for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_response(reply);
		return ret;
	}
	inline const logon_info& ftp_client::last_logon_info() const {
		return last_logon_info_;
	}
	inline bool ftp_client::logout() {
		bool ret = send_command_with_check("QUIT");
		close_control_channel();
		return ret;
	}
	inline bool ftp_client::get_single_response_line(std::string& response) const {
		if( !is_connected() )
			return false;

		if( reponse_queue.empty() ) {
			// internal buffer is empty ==> get response from ftp-server
			int num=0;
			std::string temp;

			do {
				ACE_Time_Value t(timeout_);
				t += ACE_OS::gettimeofday();
				num=socket_->recv((void*)buffer_, sizeof(buffer_)-1, &t);
				if( response_wait_ !=0 )
					ACE_OS::sleep(response_wait_);
				buffer_[num] = '\0';
				temp+=buffer_;
			} while( num==sizeof(buffer_)-1 && num!=0 && num!=-1 );
			if( num==0 ) {
				report_error("Receive", __FILE__, __LINE__);
				socket_->close();
				return false;
			}
			// each line in response is a separate entry in the internal buffer
			while( temp.length() ) {
				size_t CRLF=temp.find('\n');
				if( CRLF != std::string::npos ) {
					reponse_queue.push(temp.substr(0, CRLF+1));
					temp.erase(0, CRLF+1);
				} else {
					// this is not rfc standard; normally each command must end with CRLF
					// in this case it doesn't
					reponse_queue.push(temp);
					temp.clear();
				}
			}

			if( reponse_queue.empty() )
				return false;
		}

		// get first response-line from buffer
		response = reponse_queue.front();
		reponse_queue.pop();

		// remove CrLf if exists
		size_t i = response.length();
		if( i>=1 && response[i-1] == '\n' ) {
			response[i-1] = '\0';
			if( i>=2 && response[i-2]=='\r' ) {
				response[i-2] = '\0';
			}
		}
		return true;
	}
	inline void ftp_client::report_error(const std::string& error_msg, const std::string& filename, 
										 int line_num) const {
		for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); it++ )
			(*it)->on_internal_error(error_msg, filename, line_num);
	}
	inline bool ftp_client::open_control_channel(const std::string& hostname, unsigned short int port) {
		close_control_channel();
		ACE_SOCK_Connector conn;
		ACE_INET_Addr addr(port, hostname.c_str());
		socket_.reset( new ACE_SOCK_Stream );

		return conn.connect(*socket_, addr)==0;
	}
	inline void ftp_client::close_control_channel() {
		if( is_connected() ) {
			socket_->close();
		}
	}
	inline bool ftp_client::send_command(const std::string& cmd) const {
		if( !is_connected() )
			return false;

		for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_send_command(cmd);
		ACE_Time_Value t(timeout_);
		t += ACE_OS::gettimeofday();
		int ret = socket_->send_n((cmd + "\r\n").c_str(), cmd.length()+2, &t);
		if( ret==-1 ) {
			report_error("Send", __FILE__, __LINE__);
			socket_->close();
			return false;
		}
		return true;
	}
	inline bool ftp_client::send_command(const std::string& cmd, ftp_reply& reply) const {
		if( !send_command(cmd) || !get_response(reply) )
			return false;
		return true;
	}
	inline bool ftp_client::send_command_with_check(const std::string& cmd) const {
		ftp_reply reply;
		if( !send_command(cmd, reply) )
			return false;
		return simple_error_check(reply);
	}
	inline bool ftp_client::cd(const std::string& dir) const {
		return send_command_with_check("CWD "+dir);
	}

	inline bool ftp_client::simple_error_check(const ftp_reply& reply) const {
		if( reply.code().is_negative_reply() )
			return false;
		else if( reply.code().is_positive_completion_reply() )
			return true;
		assert( reply.code().is_positive_reply() );
		return false;
	}
	inline ftp_client::~ftp_client() {
		if( is_transferring_data() )
			abort();

		if( is_connected() )
			logout();
	}
	inline bool ftp_client::abort() const {
		if( trans_now_ ) {
			abort_trans_ = true;
			return true;
		}

		abort_trans_ = false;
		ftp_reply reply;
		if( !send_command("ABOR", reply) )
			return false;
		return simple_error_check(reply);
	}
	inline bool ftp_client::system() const {
		return send_command_with_check("SYST");
	}
	inline bool ftp_client::noop() const {
		return send_command_with_check("NOOP");
	}
	inline bool ftp_client::pwd() const {
		return send_command_with_check("PWD");
	}
	inline bool ftp_client::cdup() const {
		return send_command_with_check("CDUP");
	}
	inline bool ftp_client::rmdir(const std::string& dir) const {
		return send_command_with_check("RMD "+dir);
	}
	inline bool ftp_client::mkdir(const std::string& dir) const {
		return send_command_with_check("MKD "+dir);
	}
	inline bool ftp_client::rm(const std::string& filename) const {
		return send_command_with_check("DELE "+filename);
	}
	inline bool ftp_client::site(const std::string& cmd) const {
		return send_command_with_check("SITE "+cmd);
	}
	inline bool ftp_client::help() const {
		return send_command_with_check("HELP");
	}
	inline bool ftp_client::help(const std::string& topic) const {
		return send_command_with_check("HELP "+topic);
	}
	inline bool ftp_client::file_structure(FILE_STRUCTURE fs) const {
		switch( fs ) {
		case FS_FILE:
			return send_command_with_check("STRU F");
		case FS_RECORD:
			return send_command_with_check("STRU R");
		case FS_PAGE:
			return send_command_with_check("STRU P");
		}
	}
	inline bool ftp_client::transfer_mode(TRANSFER_MODE mode) const {
		switch( mode ) {
		case TM_STREAM:
			return send_command_with_check("MODE S");
		case TM_BLOCK:
			return send_command_with_check("MODE B");
		case TM_COMPRESSED:
			return send_command_with_check("MODE C");
		}
	}
	inline bool ftp_client::status() const {
		return send_command_with_check("STAT");
	}
	inline bool ftp_client::status(const std::string& path) const {
		return send_command_with_check("STAT "+path);
	}
	inline bool ftp_client::restart(int pos) const {
		std::ostringstream os;
		os << "REST " << pos;
		ftp_reply reply;
		if( !send_command(os.str(), reply) )
			return false;

		if( reply.code().is_positive_intermediate_reply() )
			return true;
		else if( reply.code().is_negative_reply() )
			return false;

		assert( reply.code().is_positive_reply() );
		return false;
	}
	inline bool ftp_client::reinitialize() const {
		ftp_reply reply;
		if( !send_command("REIN", reply) )
			return false;

		if( reply.code().is_positive_completion_reply() )
			return true;
		else if( reply.code().is_positive_preliminary_reply() ) {
			if( !get_response(reply) || !reply.code().is_positive_completion_reply() )
				return false;
		} else if( reply.code().is_negative_reply() )
			return false;

		assert( reply.code().is_positive_intermediate_reply() );
		return false;
	}
	inline bool ftp_client::file_size(const std::string& filename, long& size) const {
		ftp_reply reply;
		if( !send_command("SIZE "+filename, reply ) )
			return false;
		size = atol(reply.value().substr(4).c_str());
		return simple_error_check(reply);
	}
	inline bool ftp_client::file_modify_time(const std::string& filename, std::string& mod_time) const {
		mod_time.erase();
		ftp_reply reply;
		if( !send_command("MDTM "+filename, reply) )
			return false;

		if( reply.value().length()>=18 ) {
			std::string tmp(reply.value().substr(4));
			size_t pos=tmp.find('.');
			if( pos!=std::string::npos )
				tmp = tmp.substr(0, pos);
			if( tmp.length()==14 )
				mod_time=tmp;
		}
		if( mod_time.empty() )
			return false;
		return simple_error_check(reply);
	}
	inline bool ftp_client::rename(const std::string& old_name, const std::string& new_name) const {
		ftp_reply reply;
		if( !send_command("RNFR "+old_name, reply) )
			return false;

		if( reply.code().is_negative_reply() )
			return false;
		else if( !reply.code().is_positive_intermediate_reply() ) {
			assert( reply.code().is_positive_completion_reply() || reply.code().is_positive_preliminary_reply() );
			return false;
		}

		if( !send_command("RNTO "+new_name, reply) )
			return false;
		return simple_error_check(reply);
	}
	inline bool ftp_client::port(const std::string& ip, unsigned short int port) const {
		std::string cmd;
		// convert the port number to 2 bytes + add to the local IP
		std::ostringstream os;
		os << "PORT " << ip << ',' << (port>>8) << ',' <<   (port&0xFF);
		cmd = os.str();
		for( size_t i=0; i<cmd.length(); ++i ) {
			if( cmd[i]=='.' ) {
				cmd[i]=',';
			}
		}
		return send_command_with_check(cmd);
	}
	inline bool ftp_client::passive(ACE_INET_Addr& addr) const {
		ftp_reply reply;
		if( !send_command("PASV", reply) )
			return false;
		if( reply.code().is_positive_completion_reply() ) {
			char buf[32];
			std::string::size_type i = reply.value().find('(');
			if( i==std::string::npos ) {
				return false;
			}
			std::string::size_type i2 = reply.value().find(')');
			if( i2==std::string::npos ) {
				return false;
			}
			strcpy(buf, reply.value().substr(i+1, i2-i-1).c_str());
			unsigned short int port=0;
			int count=0;
			char* q=0;
			for( char* p=buf; p<buf+32; ++p ) {
				if( *p==',' ) {
					++count;
					if( count==4 ) {
						*p='\0';
						q = p+1;
					} else if( count<4 ) {
						*p='.';
					} else if( count==5 ) {
						*p='\0';
						port = (unsigned short int)(atoi(q)<<8);
						q=p+1;
						break;
					}
				}
			}
			if( count==5 ) {
				port |= (unsigned short int)(atoi(q)&0xFF);
				if( addr.set(port, buf) == -1 )
					return false;
			} else {
				return false;
			}
		}
		return simple_error_check(reply);
	}
	inline bool ftp_client::open_active_data_connection(ACE_SOCK_Stream& stream, DATA_CHANNEL_CMD cmd, 
														const std::string& path, int offset) const {
		ACE_SOCK_Acceptor acceptor;
		ACE_INET_Addr addr((unsigned short int)0, (ACE_UINT32)INADDR_ANY);
		if( acceptor.open(addr) == -1 ) {
			report_error("active data connection", __FILE__, __LINE__);
		}
		acceptor.get_local_addr(addr);
		ACE_INET_Addr addr2;
		socket_->get_local_addr(addr2);
		// transmit the socket (ip address + port) to the server
		// the ftp server establishes then the data connection
		if( port(addr2.get_host_addr(), addr.get_port_number())!=true )
			return false;

		// if resuming is activated then set offset
		if( resume_if_possible_ && 
			(cmd==CMD_STOR || cmd==CMD_RETR || cmd==CMD_APPE ) &&
			(offset!=0 && restart(offset)!=true) )
			return false;

		// send FTP command RETR/STOR/NLST/LIST to the server
		ftp_reply reply;
		if( !send_command(get_cmd_str(cmd, path), reply) ||
			!reply.code().is_positive_preliminary_reply() )
			return false;

		// accept the data connection
		if( acceptor.accept(stream) == -1 ) {
			return false;
		}
		acceptor.close();
		return true;
	}
	inline bool ftp_client::open_passive_data_connection(ACE_SOCK_Stream& stream, DATA_CHANNEL_CMD cmd,
														 const std::string& path, int offset) const {
		ACE_INET_Addr addr;

		// set passive mode
		// the ftp server opens a port and tell us the socket (ip address + port)
		// this socket is used for opening the data connection 
		if( passive(addr)!=true )
			return false;
		ACE_SOCK_Connector conn;
		if( conn.connect(stream, addr) == -1 ) {
			report_error("passive data connection", __FILE__, __LINE__);
			return false;
		}

		// if resuming is activated then set offset
		if( resume_if_possible_ && 
			(cmd==CMD_STOR || cmd==CMD_RETR || cmd==CMD_APPE ) &&
			(offset!=0 && restart(offset)!=true) )
			return false;

		// send FTP command RETR/STOR/NLST/LIST to the server
		ftp_reply reply;
		if( !send_command(get_cmd_str(cmd, path), reply) ||
			!reply.code().is_positive_preliminary_reply() )
			return false;

		return true;
	}
	inline std::string ftp_client::get_cmd_str(DATA_CHANNEL_CMD cmd, const std::string& path) const {
		switch( cmd ) {
		case CMD_LIST: return path.empty()?"LIST":"LIST " + path;
		case CMD_NLST: return path.empty()?"NLST":"NLST " + path;
		case CMD_STOR: return "STOR " + path;
		case CMD_STOU: return "STOU " + path;
		case CMD_RETR: return "RETR " + path;
		case CMD_APPE: return "APPE " + path;
		default:
			assert( false );
		}
		return "";
	}
	inline bool ftp_client::send_data(trans_notification* observer, ACE_SOCK_Stream& stream) const {
		trans_now_=true;
		size_t bytes_readed=0;
		observer->on_pre_bytes_send(buffer_, sizeof(buffer_), bytes_readed);
		while( !abort_trans_ && bytes_readed!=0 ) {
			ACE_Time_Value t(timeout_);
			t += ACE_OS::gettimeofday();
			int ret = stream.send_n(buffer_, bytes_readed, &t);
			if( ret == -1 || ret == 0 ) {
				trans_now_=false;
				report_error("send data", __FILE__, __LINE__);
				return false;
			}

			for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
				(*it)->on_bytes_sent(buffer_, bytes_readed);

			observer->on_pre_bytes_send(buffer_, sizeof(buffer_), bytes_readed);
		}
		trans_now_=false;
		if( abort_trans_ ) {
			abort();
			return false;
		}
		return true;
	}

	inline bool ftp_client::receive_data(trans_notification* observer, ACE_SOCK_Stream& stream) const {
		trans_now_ = true;

		for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_begin_receiving_data();

		ACE_Time_Value t(timeout_);
		t += ACE_OS::gettimeofday();
		int bytes_readed=0;
		long total_bytes = 0;
		while( !abort_trans_ ) {
			bytes_readed=stream.recv(buffer_, sizeof(buffer_), &t);
			if( bytes_readed==0 ) {
				break;
			} else if( bytes_readed ==-1 ) {
				trans_now_=false;
				report_error("send data", __FILE__, __LINE__);
				return false;
			} else {
				total_bytes += bytes_readed;
				for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
					(*it)->on_bytes_received(buffer_, bytes_readed);

				observer->on_bytes_received(buffer_, bytes_readed);
			}

		}

		for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_end_receiving_data(total_bytes);

		trans_now_=false;
		if( abort_trans_ ) {
			abort();
			return false;
		}
		return true;
	}
	inline bool ftp_client::transfer_data(DATA_CHANNEL_CMD cmd, trans_notification* observer, 
										  ACE_SOCK_Stream& stream) const {
		switch( cmd ) {
		case CMD_STOR:
		case CMD_STOU:
		case CMD_APPE:
			return send_data(observer, stream);
		case CMD_RETR:
		case CMD_LIST:
		case CMD_NLST:
			return receive_data(observer, stream);
		default:
			assert( false );
			return false;
		}
		return true;
	}
	inline bool ftp_client::set_type(TRANS_TYPE type, FORMAT_TYPE ft, long size) const {
		std::string cmd;
		std::ostringstream os;
		switch( type ) {
		case TT_ASCII:     cmd = "TYPE A"; break;
		case TT_EBCDIC:    cmd = "TYPE E"; break;
		case TT_IMAGE:     cmd = "TYPE I"; break;
		case TT_LOCALBYTE: os << "TYPE L " << size; cmd = os.str(); break;
		default:
			assert( false );
			return false;
		}

		if( type==TT_ASCII || type==TT_EBCDIC ) {
			switch( ft ) {
			case FT_NONPRINT:         cmd += " N"; break;
			case FT_TELNET_FORMAT:    cmd += " T"; break;
			case FT_CARRIAGE_CONTROL: cmd += " C"; break;
			default:
				assert( false );
			}
		}

		ftp_reply reply;
		if( !send_command(cmd, reply) )
			return false;
		return simple_error_check(reply);
	}
	inline bool ftp_client::execute_data_command(DATA_CHANNEL_CMD cmd, const std::string& path, 
												 TRANS_TYPE type, FORMAT_TYPE observers_, bool is_pasv,
												 long offset, trans_notification* observer) const {
		if( trans_now_ || !is_connected() )
			return false;

		if( set_type(type, observers_)!=true )
			return false;

		ACE_SOCK_Stream stream;
		if( is_pasv ) {
			if( !open_passive_data_connection(stream, cmd, path, offset) )
				return false;
		} else {
			if( !open_active_data_connection(stream, cmd, path, offset) )
				return false;
		}
		bool trans_result = transfer_data(cmd, observer, stream);
		stream.close();

		// get response from ftp server
		ftp_reply reply;
		if( !trans_result || !get_response(reply) || !reply.code().is_positive_completion_reply() )
			return false;
		return true;
	}
	inline bool ftp_client::download_file(const std::string& remote_filename, 
										  const std::string& local_filename, 
										  TRANS_TYPE type,
										  bool is_pasv) const {
		file_trans file;
		if( !file.open(local_filename.c_str(), resume_if_possible_?"ab":"wb") ) {
			report_error("file create", __FILE__, __LINE__);
			return false;
		}
		file.seek(0, file_trans::OR_END);

		long filesize = 0;
		file_size(remote_filename, filesize);

		for( observer_set::const_iterator it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_pre_receive_file(remote_filename, local_filename, filesize);

		const bool ret=execute_data_command(CMD_RETR, remote_filename, type, FT_NONPRINT, 
											is_pasv, file.tell(), &file);

		for( observer_set::const_iterator it2=observers_.begin(); it2!=observers_.end(); it2++ )
			(*it2)->on_post_receive_file(remote_filename, local_filename, filesize);

		return ret;
	}
	inline bool ftp_client::upload_file(const std::string& local_filename, 
										const std::string& remote_filename, 
										bool store_unique, 
										TRANS_TYPE type,
										bool is_pasv) const {
		file_trans file;
		if( !file.open(local_filename.c_str(), "rb") ) {
			report_error("file read", __FILE__, __LINE__);
			return false;
		}

		long filesize = 0;
		if( resume_if_possible_ )
			file_size(remote_filename, filesize);

		DATA_CHANNEL_CMD cmd=CMD_STOR;
		if( filesize > 0 )
			cmd = CMD_APPE;
		else if( store_unique )
			cmd = CMD_STOU;

		file.seek(0, file_trans::OR_END);
		long local_filesize = file.tell();
		file.seek(filesize, file_trans::OR_BEGIN);

		observer_set::const_iterator it;
		for( it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_pre_send_file(local_filename, remote_filename, local_filesize);

		const bool ret=execute_data_command(cmd, remote_filename, type, FT_NONPRINT, 
											is_pasv, 0, &file);

		for( it=observers_.begin(); it!=observers_.end(); ++it )
			(*it)->on_post_send_file(local_filename, remote_filename, local_filesize);

		return ret;
	}
	inline bool ftp_client::ll(const std::string& path, std::vector<std::string>& file_list, bool is_pasv) const {
		string_trans st;
		if( !execute_data_command(CMD_LIST, path, TT_ASCII, FT_NONPRINT, is_pasv, 0, &st) )
			return false;

		file_list.clear();
		std::string line;
		while( st.get_next_line(line) )
			file_list.push_back(line);

		return true;
	}

	inline bool ftp_client::ls(const std::string& path, std::vector<std::string>& file_list, bool is_pasv) const {
		string_trans st;
		if( !execute_data_command(CMD_NLST, path, TT_ASCII, FT_NONPRINT, is_pasv, 0, &st) )
			return false;

		file_list.clear();
		std::string line;
		while( st.get_next_line(line) )
			file_list.push_back(line);

		return true;
	}
	inline bool ftp_client::ll(const std::string& path, std::vector<ftp_file_stat>& file_list, 
								bool is_pasv) const {
		string_trans st;
		if( !execute_data_command(CMD_LIST, path, TT_ASCII, FT_NONPRINT, is_pasv, 0, &st) )
			return false;

		file_list.clear();
		std::string line;
		ftp_list_parse parser;
		while( st.get_next_line(line) ) {
			ftp_file_stat my_stat;
			if( parser.parse(line, my_stat) ) {
				my_stat.pathname = path;
				file_list.push_back(my_stat);
			}
		}

		return true;
	}

// ftp_file_stat definition <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	inline ftp_file_stat::ftp_file_stat():pathname(""),filename(""),file_size(0),
	attr(""),uid(""),gid(""),is_dir(false),is_link(false) {
	}
// ftp_list_parse definition <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	/// Easily Parsed LIST Format (EPLF)
	/// see http://pobox.com/~djb/proto/eplf.txt
	/// "+i8388621.29609,m824255902,/,\tdev"
	/// "+i8388621.44468,m839956783,r,s10376,\tRFCEPLF"
	inline bool ftp_list_parse::try_parse_EPLF(const std::string& line, ftp_file_stat& file_stat) {
		if( line[0] != '+' )
			return false;
		long temp=0;
		int i = 1;
		const char* p = line.c_str();
		const char* end = p+line.length();
		++p;
		const char* q = p;
		for( ;p<end; ++p ) {
			if( *p == '\t' ) {
				file_stat.filename = p+1;
				return true;
			}

			if( *p == ',' ) {
				switch( *q ) {
				case '/':
					file_stat.is_dir=true;
					break;
				case 'r':
					file_stat.is_dir=false;
					break;
				case 's':
					file_stat.file_size = atol(std::string(q+1, p-q-1).c_str());
					break;
				case 'm':
					file_stat.mtime.set((time_t)atol(std::string(q+1, p-q-1).c_str()));
					break;
				}
				q=p+1;
			}
		}
		return false;
	}
	/// UNIX-style listing, without inum and without blocks
	/// "-rw-r--r--   1 root     other        531 Jan 29 03:26 README"
	/// "dr-xr-xr-x   2 root     other        512 Apr  8  1994 etc"
	/// "dr-xr-xr-x   2 root     512 Apr  8  1994 etc"
	/// "lrwxrwxrwx   1 root     other          7 Jan 25 00:17 bin -> usr/bin"
	/// 
	/// Also produced by Microsoft's FTP servers for Windows:
	/// "----------   1 owner    group         1803128 Jul 10 10:18 ls-lR.Z"
	/// "d---------   1 owner    group               0 May  9 19:45 Softlib"
	/// 
	/// Also WFTPD for MSDOS:
	/// "-rwxrwxrwx   1 noone    nogroup      322 Aug 19  1996 message.ftp"
	/// 
	/// Also NetWare:
	/// "d [R----F--] supervisor            512       Jan 16 18:53    login"
	/// "- [R----F--] rhesus             214059       Oct 20 15:27    cx.exe"
	//
	/// Also NetPresenz for the Mac:
	/// "-------r--         326  1391972  1392298 Nov 22  1995 MegaPhone.sit"
	/// "drwxrwxr-x               folder        2 May 10  1996 network"
	inline bool ftp_list_parse::try_parse_unix_style(const std::string& line, ftp_file_stat& file_stat) {
		const char* p = line.c_str();
		const char* end = p+line.length();
		switch( *p ) {
		case 'b':
		case 'c':
		case 'd':
			file_stat.is_dir = true;break;
		case 'l':
			file_stat.is_link = true;break;
		case 'p':
		case 's':
		case '-':
			break;
		default:
			return false;
		}

		int  state  = 1;

		long year   = 0;
		long month  = 0;
		long mday   = 0;
		long hour   = 0;
		long minute = 0;

		const char* p_1 = (p++);
		const char* q = p;
		for( p; p<end; p_1=p++ ) {
			if( *p==' ' && *p_1!=' ' ) {
				switch( state ) {
				case 1:	  // attribe
					file_stat.attr.assign(q-1, p);
					state = 2;
					break;
				case 2:	  // skipping nlink
					state = 3;
					if( p-q==6 && *q=='f' )	  // for NetPresenz "folder"
						state = 4;
					break;
				case 3:	  // get uid
					state = 4;
					file_stat.uid.assign(q, p);
					break;
				case 4:	  // getting size, gid
					if( !get_long(q, p-q, file_stat.file_size) ) {
						file_stat.gid.assign(q, p);
					}
					state = 5;
					break;
				case 5:	  // searching for month, otherwise getting tentative size
					month = get_month(q, p-q);
					if( month >= 0 )
						state = 6;
					else {
						get_long(q, p-q, file_stat.file_size);
					}
					break;
				case 6:	  // have size and month
					get_long(q, p-q, mday);
					state = 7;
					break;
				case 7:	  // have size, month, mday
					if( p-q==4 && q[1]==':' ) {
						get_long(q, 1, hour);
						get_long(q+2, 2, minute);
					} else if( p-q==5 && q[2]==':' ) {
						get_long(q, 2, hour);
						get_long(q+3, 2, minute);
					} else if( p-q >= 4 ) {
						get_long(q, p-q, year);
					} else
						return false;

					file_stat.filename.assign(p+1);
					state = 8;
					break;
				case 8:	  // twiddling thumbs
					break;
				}
				q = p+1;
			} else if( *p!=' ' && *p_1==' ' ) {
				q = p;
			}
		}

		if( state != 8 )
			return false;
		if( year==0 ) {
			year = wuya::datetime::current_time().year();
		}
		file_stat.mtime.set((int)year,(int)(month+1), (int)mday, (int)hour, (int)minute);
		// handle links
		if( file_stat.is_link ) {
			std::string::size_type pos = file_stat.filename.find(" -> ");
			if( pos != std::string::npos ) {
				file_stat.linkname = file_stat.filename.substr(pos+4);
				file_stat.filename = file_stat.filename.substr(0, pos);
			}

		}

		// eliminate extra NetWare spaces
		if( line[1]==' ' || line[1]=='[' ) {
			if( file_stat.filename.length()>3 && file_stat.filename.substr(0, 3)=="   " )
				file_stat.filename = file_stat.filename.substr(3);
		}

		return true;
	}
	/// MultiNet (some spaces removed from examples)
	/// "00README.TXT;1      2 30-DEC-1996 17:44 [SYSTEM] (RWED,RWED,RE,RE)"
	/// "CORE.DIR;1          1  8-SEP-1996 16:09 [SYSTEM] (RWE,RWE,RE,RE)"
	/// and non-MutliNet VMS:
	/// "CII-MANUAL.TEX;1  213/216  29-JAN-1996 03:33:12  [ANONYMOU,ANONYMOUS]   (RWED,RWED,,)"
	inline bool ftp_list_parse::try_parse_multi_net(const std::string& line, ftp_file_stat& file_stat) {
		std::string::size_type i;
		if( (i=line.find(';'))==std::string::npos ) {
			return false;
		}
		file_stat.filename.assign(line, 0, i);
		if( file_stat.filename.find(".DIR")!=std::string::npos ) {
			file_stat.filename = file_stat.filename.substr(0, file_stat.filename.length()-4);
			file_stat.is_dir = true;
		}
		const char* p = line.c_str()+i+1;
		const char* end = line.c_str()+line.length();
		const char* q = p;
		int state = 0;
		long year   = 0;
		long month  = 0;
		long mday   = 0;
		long hour   = 0;
		long minute = 0;
		for( ;p<end;++p ) {
			if( *p=='-' || *p==':' || (*p==' ' && (state==3||state==5)) ) {
				switch( state ) {
				case 0:
					get_long(q, p-q, mday);
					q = p+1;
					state=1;
					break;
				case 1:
					month = get_month(q, p-q);
					if( month<0 ) {
						return false;
					}
					++month;
					q = p+1;
					state = 3;
					break;
				case 3:
					get_long(q, p-q, year);
					q = p+1;
					state=4;
					break;
				case 4:
					get_long(q, p-q, hour);
					q = p+1;
					state=5;
					break;
				case 5:
					get_long(q, p-q, minute);
					state=6;
					break;
				}
			} else if( *p!=' ' && *(p-1)==' ' ) {
				q = p;
			}
			if( state == 6 ) {
				break;
			}
		}
		file_stat.mtime.set((int)year, (int)month, (int)mday, (int)hour, (int)minute);
		return true;
	}
	/// MSDOS format
	/// 04-27-00  09:09PM       <DIR>          licensed
	/// 07-18-00  10:16AM       <DIR>          pub
	/// 04-14-00  03:47PM                  589 readme.htm
	inline bool ftp_list_parse::try_parse_msdos_style(const std::string& line, ftp_file_stat& file_stat) {
		const char* p = line.c_str();
		if(*p<0 || !isdigit(*p) ) {
			return false;
		}
		const char* end = line.c_str()+line.length();
		const char* q = p;
		int state = 0;
		long year   = 0;
		long month  = 0;
		long mday   = 0;
		long hour   = 0;
		long minute = 0;
		for( ;p<end;++p ) {
			if( *p=='-' || *p==':' || (*p==' ' && *(p-1)!=' ') 
				|| (state==5 && (*p=='A'||*p=='P')) 
			  ) {
				switch( state ) {
				case 0:
					get_long(q, p-q, month);
					q = p+1;
					state=1;
					break;
				case 1:
					get_long(q, p-q, mday);
					q = p+1;
					state = 3;
					break;
				case 3:
					get_long(q, p-q, year);
					q = p+1;
					state=4;
					break;
				case 4:
					get_long(q, p-q, hour);
					q = p+1;
					state=5;
					break;
				case 5:
					get_long(q, p-q, minute);
					if( *p=='P' ) {
						hour+=12;
					}
					q = p+1;
					state=6;
					break;
				case 6:
					get_long(q, p-q, file_stat.file_size);
					state = 7;
					break;
				}
			} else if( *p=='<' ) {
				file_stat.is_dir = true;
			} else if( *p!=' ' && *(p-1)==' ' ) {
				q = p;
			}
		}
		year+=(wuya::datetime::current_time().year()/100)*100;
		file_stat.mtime.set((int)year, (int)month, (int)mday, (int)hour, (int)minute);
		file_stat.filename.assign(q);
		return true;
	}
	inline bool ftp_list_parse::parse(const std::string& line, ftp_file_stat& file_stat) {
		if( line.length() < 2 )	// an empty name in EPLF, with no info, could be 2 chars
			return false;
		if( try_parse_EPLF(line, file_stat)
			|| try_parse_unix_style(line, file_stat)
			|| try_parse_multi_net(line, file_stat)
			|| try_parse_msdos_style(line, file_stat) ) {
			return true;
		}
		return false;
	}

// 0-11
	inline bool ftp_list_parse::check_month(const char* buffer, const char* month_name) const {
		if( tolower(buffer[0]) != month_name[0] ||
			tolower(buffer[1]) != month_name[1] ||
			tolower(buffer[2]) != month_name[2] )
			return false;
		return true;
	}

	inline int ftp_list_parse::get_month(const char* buffer,int len) const {
		static const char* MONTH[12]={
			"jan","feb","mar","apr","may","jun",
			"jul","aug","sep","oct","nov","dec"
		};
		if( len == 3 ) {
			for( int i = 0; i < 12; ++i ) {
				if( check_month(buffer, MONTH[i]) )
					return i;
			}
		}
		return -1;
	}

	inline bool ftp_list_parse::get_long(const char* str, int len, long& value) const {
		char* end_ptr = 0;
		std::string buf(str, len);
		value = strtol(buf.c_str(), &end_ptr, 10);
		if( end_ptr!=0 && end_ptr[0]!='\0' ) {
			return false;
		}
		return true;
	}
}
#endif // __FTP_CLIENT_H__
