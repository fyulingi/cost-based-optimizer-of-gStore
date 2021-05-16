#ifndef _UTIL_TRANSACTION_H
#define _UTIL_TRANSACTION_H

#include "../Util/Util.h"
#include "../Util/IDTriple.h"

using namespace std;

typedef unsigned long long txn_id_t;
typedef set<IDTriple> TripleSet;
typedef set<TYPE_ENTITY_LITERAL_ID> IDSet;
typedef set<TYPE_TXN_ID> TXNSet;
enum class TransactionState{ WAITING, RUNNING, COMMITTED, ABORTED};
enum class IsolationLevelType {
	INVALID = INVALID_TYPE_ID,
	SERIALIZABLE = 1,      // serializable
	SNAPSHOT = 2,          // snapshot isolation
	READ_COMMITTED = 3,    // read committed
};

class Transaction {
private:
	TYPE_TXN_ID TID;
	TYPE_TXN_ID CommitID;
	string db_name;

	TransactionState state;
	TYPE_TS timestamp;

	long int start_time;
	long int end_time;
	
	//latch table
	vector<IDSet> ReadSet; //shared latches
	vector<IDSet> WriteSet; //exclusive locks and latchess
	
	//TXNSet DependedTXNSet;
	
	vector<string> sparqls;
	IsolationLevelType isolation;
public:
	unsigned long long int update_num;
	enum class IDType{SUBJECT, PREDICATE, OBJECT};
	Transaction(Transaction const&) = delete;
	Transaction(string name, TYPE_TS time, txn_id_t TID, IsolationLevelType _isolation = IsolationLevelType::SERIALIZABLE);
	~Transaction(){
	};
	
	
	inline TransactionState GetState() const { return this->state;  }
	inline void SetState(TransactionState _state) { this->state = _state; }
	inline void SetEndTime(long long int time) { end_time = time; }
	inline long GetStartTime() const { return start_time;  }
	inline long GetEndTime() const { return end_time;  }
	
	//inline void SetTimeStamp(TYPE_TS _timestamp) {this->timestamp = _timestamp; }
	//inline TYPE_TS GetTimeStamp() const {return this->timestamp; };
	
	TYPE_TXN_ID GetTID() const { return this->TID; }
	TYPE_TXN_ID GetCommitID() const { return this->CommitID; }
	
	//void SetTID(TYPE_TXN_ID _TID) { this->TID = _TID;}
	void SetCommitID(TYPE_TXN_ID _CID) {this->CommitID = _CID; }
	
	
	void ReadSetInsert(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type );
	void ReadSetDelete(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type );
	bool ReadSetFind(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type);
	
	void WriteSetInsert(IDTriple _Triple);
	void WriteSetDelete(IDTriple _Triple);
	bool WriteSetFind(TYPE_ENTITY_LITERAL_ID _ID, Transaction::IDType _type);
	
	void AddSetInsert(IDTriple _Triple);
	void AddSetDelete(IDTriple _Triple);
	void DelSetInsert(IDTriple _Triple);
	void DelSetDelete(IDTriple _Triple);
	
	//void DependedTXNSetInsert(TYPE_TXN_ID _TID);
	//void DependedTXNSetDelete(TYPE_TXN_ID _TID);
	
	IsolationLevelType GetIsolationLevelType() {return this->isolation; }
	
	//inline TripleSet& Get_AddSet() { return this->AddSet; }
	//inline TripleSet& Get_DelSet() { return this->DelSet; }
	inline vector<IDSet>& Get_WriteSet() {return this->WriteSet; }
	inline vector<IDSet>& Get_ReadSet() {return this->ReadSet; }
	
	void print_ReadSet();
	void print_WriteSet();
	void print_AddSet();
	void print_DelSet();
	void print_all();
};

#endif
