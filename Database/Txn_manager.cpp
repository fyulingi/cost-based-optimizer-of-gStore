#include "Txn_manager.h"


Txn_manager::Txn_manager(Database *db, string db_name)
{
	this->db = db;
	this->db_name = db_name;
	this->log_path = "./" + db_name + ".db/" + "update.log";
	this->all_log_path = "./" + db_name + ".db/" + "update_since_backup.log";
	out.open(this->log_path.c_str(), ios::out | ios::app);
	out_all.open(this->all_log_path.c_str(), ios::out | ios::app);
	cnt.store(1);
	txn_table.clear();
}

Txn_manager::~Txn_manager()
{
	abort_all_running();
	Checkpoint();
	txn_table.clear();
	out.close();
	out_all.close();
}

void Txn_manager::writelog(string str)
{
	this->lock_log();
	out << str << endl;
	out_all << str << endl;
	this->unlock_log();
}

bool Txn_manager::add_transaction(txn_id_t TID, shared_ptr<Transaction> txn)
{
	//assert(txn_table.find(TID) == txn_table.end());
	txn_table.insert(pair<txn_id_t,shared_ptr<Transaction> >(TID, txn));
	return true;
}

//TODO: undo failed
bool Txn_manager::undo(string str, txn_id_t TID)
{
	Util::get_timestamp(str);
	string undo_sparql;
	if (str[0] == 'I') {
		undo_sparql = "DELETE DATA{";
	}
	else if (str[0] == 'D') {
		undo_sparql = "INSERT DATA{";
	}
	else {
		cerr << "wrong undo sparql: " << str << endl;
		return false;
	}
	undo_sparql += str.substr(2, str.length());
	undo_sparql += '}';
	ResultSet rs;
	FILE* output = stdout;
	shared_ptr<Transaction> txn = txn_table[TID];
	if(db != nullptr)
		this->db->query(undo_sparql, rs, output);
	else
	{
		cout << "error! database has been flushed or removed!" << endl;
		return false;
	}
	return true;
}

bool Txn_manager::redo(string str, txn_id_t TID)
{
	Util::get_timestamp(str);
	string redo_sparql;
	if (str[0] == 'I') {
		redo_sparql = "INSERT DATA{";
	}
	else if (str[0] == 'D') {
		redo_sparql = "DELETE DATA{";
	}
	else {
		cerr << "wrong redo sparql: " << str << endl;
		return false;
	}
	redo_sparql += str.substr(2, str.length());
	redo_sparql += '}';
	ResultSet rs;
	FILE* output = stdout;
	shared_ptr<Transaction> txn = txn_table[TID]; 
	if(db != nullptr)
		this->db->query(redo_sparql, rs, output);
	else
	{
		cout << "error! database has been flushed or removed" << endl;
		return false;
	}
	return true;
}


/*
inline txn_id_t Txn_manager::ArrangeTID()
{
	srand(time(NULL));
	cnt++;
	return stod(Util::get_timestamp()) * 10000 + (rand() + cnt) % 10000 ;
}
*/

inline txn_id_t Txn_manager::ArrangeTID()
{
	txn_id_t TID = cnt.load();
	if(TID == INVALID_ID)
	{
		cout << "TID wrapped! " << endl;
	}
	cnt++;
	return TID;
}

inline txn_id_t Txn_manager::ArrangeCommitID()
{
	return cnt.load();
}

txn_id_t Txn_manager::Begin(IsolationLevelType isolationlevel)
{
	checkpoint_lock.lockShared();
	txn_id_t TID = this->ArrangeTID();
	if(TID == INVALID_ID)
	{
		cout << "TID wrapped, please run garbage clean!" << endl;
		checkpoint_lock.unlock();
		return TID;
	}
	shared_ptr<Transaction> txn = make_shared<Transaction>(this->db_name, Util::get_cur_time(), TID, isolationlevel);
	txn->SetCommitID(TID);
	add_transaction(TID, txn);
	string log_str = "Begin " + Util::int2string(TID);
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		checkpoint_lock.unlock();
		return -1;
	}
	//writelog(log_str);
	txn_table[TID]->SetState(TransactionState::RUNNING);
	return TID;
}

int Txn_manager::Commit(txn_id_t TID)
{
	string log_str = "Commit " + Util::int2string(TID);
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	else if (txn_table[TID]->GetState() != TransactionState::RUNNING) {
		cerr << "transaction not in running state! commit failed" << endl;
		return 1;
	}
	txn_id_t CID = this->ArrangeCommitID();
	txn_table[TID]->SetCommitID(CID);
	if(db != nullptr)
		db->transaction_commit(txn_table[TID]);
	else
	{
		cout << "error! database has been flushed or removed" << endl;
		return -1;
	}
	//writelog(log_str);
	txn_table[TID]->SetState(TransactionState::COMMITTED);
	txn_table[TID]->SetEndTime(Util::get_cur_time());
	checkpoint_lock.unlock();
	return 0;
}

int Txn_manager::Abort(txn_id_t TID)
{
	string log_str = "Abort " + Util::int2string(TID);
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	if(db != nullptr)
		db->transaction_rollback(txn_table[TID]);
	else
	{
		cout << "error! database has been flushed or removed" << endl;
		return -1;
	}
	//writelog(log_str);
	txn_table[TID]->SetState(TransactionState::ABORTED);
	txn_table[TID]->SetEndTime(Util::get_cur_time());
	checkpoint_lock.unlock();
	return 0;
}

int Txn_manager::Rollback(txn_id_t TID)
{
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	else if (txn_table[TID]->GetState() != TransactionState::RUNNING) {
		cerr << "transaction not in running state! rollback failed" << endl;
		return 1;
	}
	return Abort(TID);
}

int Txn_manager::Query(txn_id_t TID, string sparql, string& results)
{
	if (txn_table.find(TID) == txn_table.end()) {
		cerr << "wrong transaction id!" << endl;
		return -1;
	}
	else if (txn_table[TID]->GetState() != TransactionState::RUNNING) {
		cerr << "transaction not in running state! Query failed" << endl;;
		return -99;
	}
	shared_ptr<Transaction> txn = txn_table[TID]; 
	int ret_val;
	ResultSet rs;
	FILE* output = stdout;
	if(db != nullptr)
		ret_val = this->db->query(sparql, rs, output , true, false, txn);
	else{
		cout << "error! database has been flushed or removed" << endl;
		return -10;
	}
	if(txn->GetState() == TransactionState::ABORTED)
	{
		cerr << "Transaction Abort due to Query failed. " << endl;
		Abort(TID);
		return -20;
	}
	if (ret_val < -1)   //non-update query
	{
		results = rs.to_JSON();
		return ret_val;
	}
	else
	{
		txn_table[TID]->update_num += ret_val;
		return ret_val;
	}
}

void Txn_manager::Checkpoint()
{
	checkpoint_lock.lockExclusive();
	if(db != nullptr)
		db->version_clean();
	checkpoint_lock.unlock();
}


//TODO:this function is not complete
void Txn_manager::restore()
{
	string line;
	ifstream in;
	in.open(this->log_path.c_str(), ios::in);
	txn_id_t  TID = -1; //NOT COMPLETE
	while (getline(in, line)) 
	{
		if (line[0] == 'B')
		{
			vector<string> redo_set;
			bool full_txn = false;
			while (getline(in, line))
			{
				if (line[0] == 'B' || line[0] == 'C')
				{
					break;
				}
				else if (line[0] == 'A')
				{
					if (redo_set.size() != 0)
						continue;
					else
						break; //no update txn
				}
				else {
					redo_set.push_back(line);
				}
			}
			for (int i = 0; i < redo_set.size(); i++)
			{
				redo(redo_set[i], TID);
			}
		}
	}
}

txn_id_t Txn_manager::find_latest_txn()
{
	auto it = txn_table.begin();
	if (it == txn_table.end()) {
		return 0;
	}
	txn_id_t max = it->first;
	bool is_found = false;
	if (it->second->GetState() == TransactionState::RUNNING)
	{
		is_found = true;
	}
	for (; it != txn_table.end(); it++)
	{
		if (it->second->GetStartTime() > txn_table[max]->GetStartTime() && it->second->GetState() == TransactionState::RUNNING)
		{
			max = it->first;
			is_found = true;
		}
	}
	if (is_found)
		return max;
	else
		return 0;
}

void Txn_manager::abort_all_running()
{
	auto it = txn_table.begin();
	for (; it != txn_table.end(); it++)
	{
		if (it->second->GetState() == TransactionState::RUNNING)
		{
			Abort(it->first);
		}
	}
}

void Txn_manager::print_txn_dataset(txn_id_t TID)
{
	auto txn = Get_Transaction(TID);
	txn->print_all();
}