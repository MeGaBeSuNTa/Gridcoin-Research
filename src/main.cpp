// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "alert.h"
#include "checkpoints.h"
#include "db.h"
#include "txdb.h"
#include "net.h"
#include "init.h"
#include "ui_interface.h"
#include "kernel.h"
#include <math.h>       /* pow */
#include "scrypt.h"
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <ctime>
#include <openssl/md5.h>
#include <boost/lexical_cast.hpp>
#include "global_objects_noui.hpp"
#include "bitcoinrpc.h"
#include "util.h"
#include <boost/algorithm/string/case_conv.hpp> // for to_lower()
#include <boost/algorithm/string/predicate.hpp> // for startswith() and endswith()
#include <boost/algorithm/string/join.hpp>
#include <boost/thread.hpp>
#include "cpid.h"
#include <boost/asio.hpp>

int GetDayOfYear();
void GetBeaconElements(std::string sBeacon,std::string& out_cpid, std::string& out_address, std::string& out_publickey);
extern std::string NodeAddress(CNode* pfrom);
extern std::string ConvertBinToHex(std::string a);
extern std::string ConvertHexToBin(std::string a);
extern bool WalletOutOfSync();
extern bool WriteKey(std::string sKey, std::string sValue);
std::string GetBeaconPublicKey(std::string cpid,bool bAdvertisingBeacon);
std::string GetBeaconPrivateKey(std::string cpid);
bool AdvertiseBeacon(bool bFromService, std::string &sOutPrivKey, std::string &sOutPubKey, std::string &sError, std::string &sMessage);
std::string SignBlockWithCPID(std::string sCPID, std::string sBlockHash);
extern void CleanInboundConnections(bool bClearAll);
extern bool PushGridcoinDiagnostics();
double qtPushGridcoinDiagnosticData(std::string data);
int RestartClient();
bool RequestSupermajorityNeuralData();
extern bool AskForOutstandingBlocks(uint256 hashStart);
extern int64_t CoinFromValue(double dAmount);
extern bool CleanChain();
extern void ResetTimerMain(std::string timer_name);
extern std::string UnpackBinarySuperblock(std::string sBlock);
extern std::string PackBinarySuperblock(std::string sBlock);
extern std::vector<unsigned char> StringToVector(std::string sData);
extern bool TallyResearchAverages(bool Forcefully);
extern void IncrementCurrentNeuralNetworkSupermajority(std::string NeuralHash, std::string GRCAddress, double distance);
bool VerifyCPIDSignature(std::string sCPID, std::string sBlockHash, std::string sSignature);
int DownloadBlocks();
int DetermineCPIDType(std::string cpid);
extern MiningCPID GetInitializedMiningCPID(std::string name, std::map<std::string, MiningCPID>& vRef);
extern std::string getHardDriveSerial();
extern bool IsSuperBlock(CBlockIndex* pIndex);
extern bool VerifySuperblock(std::string superblock, int nHeight);
extern double ExtractMagnitudeFromExplainMagnitude();
extern void AddPeek(std::string data);
extern void GridcoinServices();
int64_t BeaconTimeStamp(std::string cpid, bool bZeroOutAfterPOR);
extern bool NeedASuperblock();
extern double SnapToGrid(double d);
extern bool NeuralNodeParticipates();
extern bool StrLessThanReferenceHash(std::string rh);
void BusyWaitForTally();
extern bool TallyNetworkAverages(bool Forcefully);
extern void SetUpExtendedBlockIndexFieldsOnce();
extern bool IsContract(CBlockIndex* pIndex);
std::string ExtractValue(std::string data, std::string delimiter, int pos);
extern bool IsSuperBlock(CBlockIndex* pIndex);
extern MiningCPID GetBoincBlockByIndex(CBlockIndex* pblockindex);
json_spirit::Array MagnitudeReport(std::string cpid);
extern void AddCPIDBlockHash(const std::string& cpid, const uint256& blockhash);
extern void ZeroOutResearcherTotals(std::string cpid);
extern StructCPID GetLifetimeCPID(std::string cpid,std::string sFrom);
extern std::string getCpuHash();
std::string getMacAddress();
std::string TimestampToHRDate(double dtm);
std::string AddContract(std::string sType, std::string sName, std::string sContract);
bool CPIDAcidTest(std::string boincruntimepublickey);
bool CPIDAcidTest2(std::string bpk, std::string externalcpid);

std::string MyBeaconExists(std::string cpid);
extern bool BlockNeedsChecked(int64_t BlockTime);
extern void FixInvalidResearchTotals(std::vector<CBlockIndex*> vDisconnect, std::vector<CBlockIndex*> vConnect);
int64_t GetEarliestWalletTransaction();
extern void IncrementVersionCount(std::string Version);
double GetSuperblockAvgMag(std::string data,double& out_beacon_count,double& out_participant_count,double& out_avg,bool bIgnoreBeacons);
extern bool LoadAdminMessages(bool bFullTableScan,std::string& out_errors);
extern std::string VectorToString(std::vector<unsigned char> v);
extern bool UnusualActivityReport();

extern std::string GetCurrentNeuralNetworkSupermajorityHash(double& out_popularity);
extern std::string GetNeuralNetworkSupermajorityHash(double& out_popularity);
       
extern double CalculatedMagnitude2(std::string cpid, int64_t locktime,bool bUseLederstrumpf);
extern CBlockIndex* GetHistoricalMagnitude_ScanChain(std::string cpid);
extern bool IsLockTimeWithin14days(double locktime);
extern int64_t ComputeResearchAccrual(int64_t nTime, std::string cpid, std::string operation, CBlockIndex* pindexLast, bool bVerifyingBlock, int VerificationPhase, double& dAccrualAge, double& dMagnitudeUnit, double& AvgMagnitude);



extern bool UpdateNeuralNetworkQuorumData();
bool AsyncNeuralRequest(std::string command_name,std::string cpid,int NodeLimit);
double qtExecuteGenericFunction(std::string function,std::string data);
extern std::string GetQuorumHash(std::string data);
extern bool FullSyncWithDPORNodes();
extern  void TestScan();
extern void TestScan2();



std::string qtExecuteDotNetStringFunction(std::string function, std::string data);


bool CheckMessageSignature(std::string sMessageAction, std::string sMessageType, std::string sMsg, std::string sSig,std::string opt_pubkey);
bool TallyMagnitudesByContract();
bool SynchronizeRacForDPOR(bool SyncEntireCoin);
extern std::string ReadCache(std::string section, std::string key);
extern std::string strReplace(std::string& str, const std::string& oldStr, const std::string& newStr);
extern bool GetEarliestStakeTime(std::string grcaddress, std::string cpid);
extern double GetTotalBalance();
extern std::string PubKeyToAddress(const CScript& scriptPubKey);
extern void IncrementNeuralNetworkSupermajority(std::string NeuralHash, std::string GRCAddress,double distance);
extern bool LoadSuperblock(std::string data, int64_t nTime, double height);


extern CBlockIndex* GetHistoricalMagnitude(std::string cpid);

extern double GetOutstandingAmountOwed(StructCPID &mag, std::string cpid, int64_t locktime, double& total_owed, double block_magnitude);
extern StructCPID GetInitializedStructCPID2(std::string name,std::map<std::string, StructCPID>& vRef);


extern double GetOwedAmount(std::string cpid);
extern double Round(double d, int place);
extern bool ComputeNeuralNetworkSupermajorityHashes();

extern void DeleteCache(std::string section, std::string keyname);
extern void ClearCache(std::string section);
bool TallyMagnitudesInSuperblock();
extern void WriteCache(std::string section, std::string key, std::string value, int64_t locktime);

std::string qtGetNeuralContract(std::string data);

extern  std::string GetNetsoftProjects(std::string cpid);
extern std::string GetNeuralNetworkReport();
void qtSyncWithDPORNodes(std::string data);
std::string qtGetNeuralHash(std::string data);
std::string GetListOf(std::string datatype);

std::string GetCommandNonce(std::string command);
std::string DefaultBlockKey(int key_length);
void InitializeBoincProjects();

extern double Cap(double dAmt, double Ceiling);
extern std::string ToOfficialNameNew(std::string proj);

extern double GRCMagnitudeUnit(int64_t locktime);
unsigned int nNodeLifespan;

using namespace std;
using namespace boost;
std::string DefaultBoincHashArgs();

//
// Global state
//

CCriticalSection cs_setpwalletRegistered;
set<CWallet*> setpwalletRegistered;

CCriticalSection cs_main;

extern std::string NodeAddress(CNode* pfrom);
extern std::string ExtractHTML(std::string HTMLdata, std::string tagstartprefix,  std::string tagstart_suffix, std::string tag_end);

CTxMemPool mempool;
unsigned int nTransactionsUpdated = 0;
unsigned int REORGANIZE_FAILED = 0;
extern void RemoveNetworkMagnitude(double LockTime, std::string cpid, MiningCPID bb, double mint, bool IsStake);

unsigned int WHITELISTED_PROJECTS = 0;
unsigned int CHECKPOINT_VIOLATIONS = 0;
int64_t nLastTallied = 0;
int64_t nLastPing = 0;
int64_t nLastPeek = 0;
int64_t nLastAskedForBlocks = 0;
int64_t nBootup = 0;
int64_t nLastCalculatedMedianTimePast = 0;
double nLastBlockAge = 0;
int64_t nLastCalculatedMedianPeerCount = 0;
int nLastMedianPeerCount = 0;
int64_t nLastTallyBusyWait = 0;

int64_t nLastTalliedNeural = 0;
int64_t nLastLoadAdminMessages = 0;
int64_t nCPIDsLoaded = 0;
int64_t nLastGRCtallied = 0;
int64_t nLastCleaned = 0;


extern bool IsCPIDValidv3(std::string cpidv2, bool allow_investor);

std::string DefaultOrg();
std::string DefaultOrgKey(int key_length);

extern std::string boinc_hash(const std::string str);
double MintLimiter(double PORDiff,int64_t RSA_WEIGHT,std::string cpid,int64_t locktime);
extern std::string ComputeCPIDv2(std::string email, std::string bpk, uint256 blockhash);
extern double GetBlockDifficulty(unsigned int nBits);
double GetLastPaymentTimeByCPID(std::string cpid);
extern bool Contains(std::string data, std::string instring);

extern bool LockTimeRecent(double locktime);
extern double CoinToDouble(double surrogate);
extern double coalesce(double mag1, double mag2);
extern int64_t Floor(int64_t iAmt1, int64_t iAmt2);
extern double PreviousBlockAge();
void CheckForUpgrade();
int AddressUser();
int64_t GetRSAWeightByCPID(std::string cpid);
extern MiningCPID GetMiningCPID();
extern StructCPID GetStructCPID();
extern std::string GetArgument(std::string arg, std::string defaultvalue);

extern void SetAdvisory();
extern bool InAdvisory();
json_spirit::Array MagnitudeReportCSV(bool detail);

bool bNewUserWizardNotified = false;
int64_t nLastBlockSolved = 0;  //Future timestamp
int64_t nLastBlockSubmitted = 0;

uint256 muGlobalCheckpointHash = 0;
uint256 muGlobalCheckpointHashRelayed = 0;
int muGlobalCheckpointHashCounter = 0;
///////////////////////MINOR VERSION////////////////////////////////
std::string msMasterProjectPublicKey  = "049ac003b3318d9fe28b2830f6a95a2624ce2a69fb0c0c7ac0b513efcc1e93a6a6e8eba84481155dd82f2f1104e0ff62c69d662b0094639b7106abc5d84f948c0a";
// The Private Key is revealed by design, for public messages only:
std::string msMasterMessagePrivateKey = "308201130201010420fbd45ffb02ff05a3322c0d77e1e7aea264866c24e81e5ab6a8e150666b4dc6d8a081a53081a2020101302c06072a8648ce3d0101022100fffffffffffffffffffffffffffffffffffffffffffffffffffffffefffffc2f300604010004010704410479be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798483ada7726a3c4655da4fbfc0e1108a8fd17b448a68554199c47d08ffb10d4b8022100fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141020101a144034200044b2938fbc38071f24bede21e838a0758a52a0085f2e034e7f971df445436a252467f692ec9c5ba7e5eaa898ab99cbd9949496f7e3cafbf56304b1cc2e5bdf06e";
std::string msMasterMessagePublicKey  = "044b2938fbc38071f24bede21e838a0758a52a0085f2e034e7f971df445436a252467f692ec9c5ba7e5eaa898ab99cbd9949496f7e3cafbf56304b1cc2e5bdf06e";

std::string BackupGridcoinWallet();
extern double GetPoSKernelPS2();
extern std::string GetBoincDataDir2();
double GetUntrustedMagnitude(std::string cpid, double& out_owed);

extern uint256 GridcoinMultipleAlgoHash(std::string t1);
extern bool OutOfSyncByAgeWithChanceOfMining();

int RebootClient();
vector<CInv> vNotFound;

std::string YesNo(bool bin);

extern double GetGridcoinBalance(std::string SendersGRCAddress);
int64_t GetMaximumBoincSubsidy(int64_t nTime);
extern bool IsLockTimeWithinMinutes(int64_t locktime, int minutes);
extern bool IsLockTimeWithinMinutes(double locktime, int minutes);
double GetNetworkProjectCountWithRAC();
extern double CalculatedMagnitude(int64_t locktime,bool bUseLederstrumpf);
extern int64_t GetCoinYearReward(int64_t nTime);
extern void AddNMRetired(double height,double LockTime, std::string cpid, MiningCPID bb);


map<uint256, CBlockIndex*> mapBlockIndex;
set<pair<COutPoint, unsigned int> > setStakeSeen;

CBigNum bnProofOfWorkLimit(~uint256(0) >> 20); // "standard" scrypt target limit for proof of work, results with 0,000244140625 proof-of-work difficulty
CBigNum bnProofOfStakeLimit(~uint256(0) >> 20);
CBigNum bnProofOfStakeLimitV2(~uint256(0) >> 20);
CBigNum bnProofOfWorkLimitTestNet(~uint256(0) >> 16);

//Gridcoin Minimum Stake Age (16 Hours)
unsigned int nStakeMinAge = 16 * 60 * 60; // 16 hours
unsigned int nStakeMaxAge = -1; // unlimited
unsigned int nModifierInterval = 10 * 60; // time to elapse before new modifier is computed
bool bRemotePaymentsEnabled = false;
bool bOPReturnEnabled = true;
bool bOptionPaymentsEnabled = false;

// Gridcoin:
int nCoinbaseMaturity = 100;
CBlockIndex* pindexGenesisBlock = NULL;
int nBestHeight = -1;
int nLastBestHeight = -1;

uint256 nBestChainTrust = 0;
uint256 nBestInvalidTrust = 0;
uint256 hashBestChain = 0;
CBlockIndex* pindexBest = NULL;
int64_t nTimeBestReceived = 0;
CMedianFilter<int> cPeerBlockCounts(5, 0); // Amount of blocks that other nodes claim to have




map<uint256, CBlock*> mapOrphanBlocks;
multimap<uint256, CBlock*> mapOrphanBlocksByPrev;
set<pair<COutPoint, unsigned int> > setStakeSeenOrphan;

map<uint256, CTransaction> mapOrphanTransactions;
map<uint256, set<uint256> > mapOrphanTransactionsByPrev;

// Constant stuff for coinbase transactions we create:
CScript COINBASE_FLAGS;
const string strMessageMagic = "Gridcoin Signed Message:\n";

// Settings
int64_t nTransactionFee = MIN_TX_FEE;
int64_t nReserveBalance = 0;
int64_t nMinimumInputValue = 0;

std::map<std::string, std::string> mvApplicationCache;
std::map<std::string, int64_t> mvApplicationCacheTimestamp;
std::map<std::string, double> mvNeuralNetworkHash;
std::map<std::string, double> mvCurrentNeuralNetworkHash;

std::map<std::string, double> mvNeuralVersion;

std::map<std::string, StructCPID> mvDPOR;
std::map<std::string, StructCPID> mvDPORCopy;

std::map<std::string, StructCPID> mvResearchAge;
std::map<std::string, HashSet> mvCPIDBlockHashes;

enum Checkpoints::CPMode CheckpointsMode;

// Gridcoin - Rob Halford

extern std::string GetHttpPageFromCreditServerRetired(std::string cpid, bool usedns, bool clearcache);
extern std::string RetrieveMd5(std::string s1);
extern std::string aes_complex_hash(uint256 scrypt_hash);

volatile bool bNetAveragesLoaded = false;
volatile bool bTallyStarted      = false;
volatile bool bRestartGridcoinMiner = false;
volatile bool bForceUpdate = false;
volatile bool bExecuteCode = false;
volatile bool bAddressUser = false;
volatile bool bCheckedForUpgrade = false;
volatile bool bCheckedForUpgradeLive = false;
volatile bool bGlobalcomInitialized = false;
volatile bool bStakeMinerOutOfSyncWithNetwork = false;
volatile bool bDoTally = false;
volatile bool bExecuteGridcoinServices = false;
volatile bool bTallyFinished = false;
volatile bool bGridcoinGUILoaded = false;
volatile bool bRequestFromHonestNode = false;

extern bool CheckWorkCPU(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey);
extern double LederstrumpfMagnitude2(double Magnitude, int64_t locktime);
extern double cdbl(std::string s, int place);

extern double GetBlockValueByHash(uint256 hash);
extern void WriteAppCache(std::string key, std::string value);
extern std::string AppCache(std::string key);
void StartPostOnBackgroundThread(int height, MiningCPID miningcpid, uint256 hashmerkleroot, double nNonce, double subsidy, unsigned int nVersion, std::string message);
extern void LoadCPIDsInBackground();
bool SubmitGridcoinCPUWork(CBlock* pblock, CReserveKey& reservekey, double nonce);
CBlock* getwork_cpu(MiningCPID miningcpid, bool& succeeded,CReserveKey& reservekey);
extern int GetBlockType(uint256 prevblockhash);
extern bool GetTransactionFromMemPool(const uint256 &hash, CTransaction &txOut);
extern unsigned int DiffBytes(double PoBDiff);
extern int Races(int iMax1000);
int ReindexWallet();

std::string cached_getblocks_args = "";
extern bool AESSkeinHash(unsigned int diffbytes, double rac, uint256 scrypthash, std::string& out_skein, std::string& out_aes512);
std::string DefaultGetblocksCommand();
CClientUIInterface uiDog;
void ExecuteCode();
extern void CreditCheckRetired(std::string cpid, bool clearcache);
extern void ThreadCPIDs();
extern std::string GetGlobalStatus();

extern void printbool(std::string comment, bool boo);
extern bool OutOfSyncByAge();
extern std::vector<std::string> split(std::string s, std::string delim);
extern bool ProjectIsValid(std::string project);
extern std::string SerializeBoincBlock(MiningCPID mcpid);
extern MiningCPID DeserializeBoincBlock(std::string block);

extern void InitializeCPIDs();
extern void ResendWalletTransactions2();
double GetNetworkAvgByProject(std::string projectname);
extern bool IsCPIDValid_Retired(std::string cpid, std::string ENCboincpubkey);
extern bool IsCPIDValidv2(MiningCPID& mc, int height);
extern void FindMultiAlgorithmSolution(CBlock* pblock, uint256 hash, uint256 hashTaget, double miningrac);
extern std::string getfilecontents(std::string filename);
extern std::string ToOfficialName(std::string proj);
extern bool LessVerbose(int iMax1000);
extern std::string ExtractXML(std::string XMLdata, std::string key, std::string key_end);
extern void ShutdownGridcoinMiner();
extern bool OutOfSync();
extern MiningCPID GetNextProject(bool bForce);
extern void HarvestCPIDs(bool cleardata);

bool FindTransactionSlow(uint256 txhashin, CTransaction& txout,  std::string& out_errors);
std::string msCurrentRAC = "";
static boost::thread_group* cpidThreads = NULL;
//static boost::thread_group* tallyThreads = NULL;
extern void FlushGridcoinBlockFile(bool fFinalize);



///////////////////////////////
// Standard Boinc Projects ////
///////////////////////////////



 //Global variables to display current mined project in various places:
 std::string 	msMiningProject = "";
 std::string 	msMiningCPID = "";
 std::string    msPrimaryCPID = "";
 std::string    msENCboincpublickey = "";
 double      	mdMiningRAC =0;
 double         mdMiningNetworkRAC = 0;
 double			mdPORNonce = 0;
 double         mdPORNonceSolved = 0;
 double         mdLastPorNonce = 0;
 double         mdMachineTimer = 0;
 double         mdMachineTimerLast = 0;
 bool           mbBlocksDownloaded = false;
 // Mining status variables
 std::string    msHashBoinc    = "";
 std::string    msHashBoincTxId= "";
 std::string    msMiningErrors = "";
 std::string    msMiningErrors2 = "";
 std::string    msMiningErrors3 = "";
 std::string    msMiningErrors5 = "";
 std::string    msMiningErrors6 = "";
 std::string    msMiningErrors7 = "";
 std::string    msMiningErrors8 = "";
 std::string    msPeek = "";
 std::string    msLastCommand = "";

 std::string    msAttachmentGuid = "";

 std::string    msMiningErrorsIncluded = "";
 std::string    msMiningErrorsExcluded = "";
 std::string    msContracts = "";

 std::string    msRSAOverview = "";
 std::string    Organization = "";
 std::string    OrganizationKey = "";
 std::string    msNeuralResponse = "";
 std::string    msHDDSerial = "";
 //When syncing, we grandfather block rejection rules up to this block, as rules became stricter over time and fields changed

 int nGrandfather = 860000;
 int nNewIndex = 271625;
 int nNewIndex2 = 364500;

 int64_t nGenesisSupply = 340569880;

 //GPU Projects:
 std::string 	msGPUMiningProject = "";
 std::string 	msGPUMiningCPID = "";
 std::string    msGPUENCboincpublickey = "";
 std::string    msGPUboinckey = "";
 double    	    mdGPUMiningRAC = 0;
 double         mdGPUMiningNetworkRAC = 0;
 // Stats for Main Screen:
 double         mdLastPoBDifficulty = 0;
 double         mdLastDifficulty = 0;
 std::string    msGlobalStatus = "";
 std::string    msLastPaymentTime = "";
 std::string    msMyCPID = "";
 double         mdOwed = 0;

 // CPU Miner threads global vars

 volatile double nGlobalHashCounter = 0;


bool fImporting = false;
bool fReindex = false;
bool fBenchmark = false;
bool fTxIndex = false;
bool fColdBoot = true;
bool fEnforceCanonical = true;
bool fUseFastIndex = false;


 int nBestAccepted = -1;


 uint256 nBestChainWork = 0;
 uint256 nBestInvalidWork = 0;
 //Optimizing internal cpu miner:
 uint256 GlobalhashMerkleRoot = 0;
 uint256 GlobalSolutionPowHash = 0;


// Gridcoin status    *************
MiningCPID GlobalCPUMiningCPID = GetMiningCPID();
int nBoincUtilization = 0;
double nMinerPaymentCount = 0;
int nPrint = 0;
std::string sBoincMD5 = "";
std::string sBoincBA = "";
std::string sRegVer = "";
std::string sBoincDeltaOverTime = "";
std::string sMinedHash = "";
std::string sSourceBlock = "";
std::string sDefaultWalletAddress = "";


std::map<std::string, StructCPID> mvCPIDs;        //Contains the project stats at the user level
std::map<std::string, StructCPID> mvCreditNode;   //Contains the verified stats at the user level
std::map<std::string, StructCPID> mvNetwork;      //Contains the project stats at the network level
std::map<std::string, StructCPID> mvNetworkCopy;      //Contains the project stats at the network level
std::map<std::string, StructCPID> mvCreditNodeCPID;        // Contains verified CPID Magnitudes;
std::map<std::string, StructCPIDCache> mvCPIDCache; //Contains cached blocknumbers for CPID+Projects;
std::map<std::string, StructCPIDCache> mvAppCache; //Contains cached blocknumbers for CPID+Projects;
std::map<std::string, StructCPID> mvBoincProjects; // Contains all of the allowed boinc projects;
std::map<std::string, StructCPID> mvMagnitudes; // Contains Magnitudes by CPID & Outstanding Payments Owed per CPID
std::map<std::string, StructCPID> mvMagnitudesCopy; // Contains Magnitudes by CPID & Outstanding Payments Owed per CPID

std::map<std::string, int> mvTimers; // Contains event timers that reset after max ms duration iterator is exceeded

// End of Gridcoin Global vars

std::map<int, int> blockcache;
bool bDebugMode = false;
bool bPoolMiningMode = false;
bool bBoincSubsidyEligible = false;
bool bCPUMiningMode = false;




//////////////////////////////////////////////////////////////////////////////
//
// dispatching functions
//

// These functions dispatch to one or all registered wallets
  double GetGridcoinBalance(std::string SendersGRCAddress)
  {
    int nMinDepth = 1;
    int nMaxDepth = 9999999;
	if (SendersGRCAddress=="") return 0;
    set<CBitcoinAddress> setAddress;
    CBitcoinAddress address(SendersGRCAddress);
	if (!address.IsValid())
	{
		printf("Checkpoints::GetGridcoinBalance::InvalidAddress");
        return 0;
	}
	setAddress.insert(address);
    vector<COutput> vecOutputs;
    pwalletMain->AvailableCoins(vecOutputs, false);
	double global_total = 0;
    BOOST_FOREACH(const COutput& out, vecOutputs)
    {
        if (out.nDepth < nMinDepth || out.nDepth > nMaxDepth)
            continue;
        if(setAddress.size())
        {
            CTxDestination address;
            if(!ExtractDestination(out.tx->vout[out.i].scriptPubKey, address))
                continue;
            if (!setAddress.count(address))
                continue;
        }
        int64_t nValue = out.tx->vout[out.i].nValue;
        //const CScript& pk = out.tx->vout[out.i].scriptPubKey;
        //CTxDestination address;
		global_total += nValue;
    }

    return CoinToDouble(global_total);
}


void ResetTimerMain(std::string timer_name)
{
	mvTimers[timer_name] = 0;
}


bool TimerMain(std::string timer_name, int max_ms)
{
	mvTimers[timer_name] = mvTimers[timer_name] + 1;
	if (mvTimers[timer_name] > max_ms)
	{
		mvTimers[timer_name]=0;
		return true;
	}
	return false;
}

bool UpdateNeuralNetworkQuorumData()
{
			#if defined(WIN32) && defined(QT_GUI)
			    if (!bGlobalcomInitialized) return false;
				std::string errors1 = "";
     			int64_t superblock_age = GetAdjustedTime() - mvApplicationCacheTimestamp["superblock;magnitudes"];
				std::string myNeuralHash = "";
				double popularity = 0;
				std::string consensus_hash = GetNeuralNetworkSupermajorityHash(popularity);
				std::string sAge = RoundToString((double)superblock_age,0);
				std::string sBlock = mvApplicationCache["superblock;block_number"];
				std::string sTimestamp = TimestampToHRDate(mvApplicationCacheTimestamp["superblock;magnitudes"]);
				std::string data = "<QUORUMDATA><AGE>" + sAge + "</AGE><HASH>" + consensus_hash + "</HASH><BLOCKNUMBER>" + sBlock + "</BLOCKNUMBER><TIMESTAMP>"
					+ sTimestamp + "</TIMESTAMP><PRIMARYCPID>" + msPrimaryCPID + "</PRIMARYCPID></QUORUMDATA>";
				std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
				qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
				qtExecuteDotNetStringFunction("SetQuorumData",data);
				return true;
			#endif
			return false;
}

bool PushGridcoinDiagnostics()
{
		#if defined(WIN32) && defined(QT_GUI)
  			    if (!bGlobalcomInitialized) return false;
				std::string errors1 = "";
                LoadAdminMessages(false,errors1);
				std::string cpiddata = GetListOf("beacon");
				std::string sWhitelist = GetListOf("project");
				int64_t superblock_age = GetAdjustedTime() - mvApplicationCacheTimestamp["superblock;magnitudes"];
				double popularity = 0;
				std::string consensus_hash = GetNeuralNetworkSupermajorityHash(popularity);
				std::string sAge = RoundToString((double)superblock_age,0);
				std::string sBlock = mvApplicationCache["superblock;block_number"];
				std::string sTimestamp = TimestampToHRDate(mvApplicationCacheTimestamp["superblock;magnitudes"]);
				printf("Pushing diagnostic data...");
				double lastblockage = PreviousBlockAge();
				double PORDiff = GetDifficulty(GetLastBlockIndex(pindexBest, true));
				std::string data = "<WHITELIST>" + sWhitelist + "</WHITELIST><CPIDDATA>"
					+ cpiddata + "</CPIDDATA><QUORUMDATA><AGE>" + sAge + "</AGE><HASH>" + consensus_hash + "</HASH><BLOCKNUMBER>" + sBlock + "</BLOCKNUMBER><TIMESTAMP>"
					+ sTimestamp + "</TIMESTAMP><PRIMARYCPID>" + msPrimaryCPID + "</PRIMARYCPID><LASTBLOCKAGE>" + RoundToString(lastblockage,0) + "</LASTBLOCKAGE><DIFFICULTY>" + RoundToString(PORDiff,2) + "</DIFFICULTY></QUORUMDATA>";
				std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
				qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
			    double dResponse = qtPushGridcoinDiagnosticData(data);
				return true;
		#endif
		return false;
}

bool FullSyncWithDPORNodes()
{
			#if defined(WIN32) && defined(QT_GUI)

				std::string sDisabled = GetArgument("disableneuralnetwork", "false");
				if (sDisabled=="true") return false;
				// 3-30-2016 : First try to get the master database from another neural network node if these conditions occur:
				// The foreign node is fully synced.  The foreign nodes quorum hash matches the supermajority hash.  My hash != supermajority hash.
				double dCurrentPopularity = 0;
				std::string sCurrentNeuralSupermajorityHash = GetCurrentNeuralNetworkSupermajorityHash(dCurrentPopularity);
				std::string sMyNeuralHash = "";
				#if defined(WIN32) && defined(QT_GUI)
					       sMyNeuralHash = qtGetNeuralHash("");
				#endif
				if (!sMyNeuralHash.empty() && !sCurrentNeuralSupermajorityHash.empty() && sMyNeuralHash != sCurrentNeuralSupermajorityHash)
				{
					bool bNodeOnline = RequestSupermajorityNeuralData();
					if (bNodeOnline) return false;  // Async call to another node will continue after the node responds.
				}
			
				std::string errors1 = "";
                LoadAdminMessages(false,errors1);
				std::string cpiddata = GetListOf("beacon");
				std::string sWhitelist = GetListOf("project");
				int64_t superblock_age = GetAdjustedTime() - mvApplicationCacheTimestamp["superblock;magnitudes"];
				double popularity = 0;
				std::string consensus_hash = GetNeuralNetworkSupermajorityHash(popularity);
				std::string sAge = RoundToString((double)superblock_age,0);
				std::string sBlock = mvApplicationCache["superblock;block_number"];
				std::string sTimestamp = TimestampToHRDate(mvApplicationCacheTimestamp["superblock;magnitudes"]);
				std::string data = "<WHITELIST>" + sWhitelist + "</WHITELIST><CPIDDATA>"
					+ cpiddata + "</CPIDDATA><QUORUMDATA><AGE>" + sAge + "</AGE><HASH>" + consensus_hash + "</HASH><BLOCKNUMBER>" + sBlock + "</BLOCKNUMBER><TIMESTAMP>"
					+ sTimestamp + "</TIMESTAMP><PRIMARYCPID>" + msPrimaryCPID + "</PRIMARYCPID></QUORUMDATA>";
				//if (fDebug3) printf("Syncing neural network %s \r\n",data.c_str());
				std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
				qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
				qtSyncWithDPORNodes(data);
			#endif
			return true;
}



double GetPoSKernelPS2()
{
    int nPoSInterval = 72;
    double dStakeKernelsTriedAvg = 0;
    int nStakesHandled = 0, nStakesTime = 0;

    CBlockIndex* pindex = pindexBest;;
    CBlockIndex* pindexPrevStake = NULL;

    while (pindex && nStakesHandled < nPoSInterval)
    {
        if (pindex->IsProofOfStake())
        {
            dStakeKernelsTriedAvg += GetDifficulty(pindex) * 4294967296.0;
            nStakesTime += pindexPrevStake ? (pindexPrevStake->nTime - pindex->nTime) : 0;
            pindexPrevStake = pindex;
            nStakesHandled++;
        }

        pindex = pindex->pprev;
    }

    double result = 0;

    if (nStakesTime)
        result = dStakeKernelsTriedAvg / nStakesTime;

    if (IsProtocolV2(nBestHeight))
        result *= STAKE_TIMESTAMP_MASK + 1;

    return result/100;
}


std::string GetGlobalStatus()
{
	//Populate overview

	try
	{
		std::string status = "";
		double boincmagnitude = CalculatedMagnitude(GetAdjustedTime(),false);
		uint64_t nWeight = 0;
		pwalletMain->GetStakeWeight(nWeight);
		nBoincUtilization = boincmagnitude; //Legacy Support for the about screen
		double weight = nWeight/COIN;
		double PORDiff = GetDifficulty(GetLastBlockIndex(pindexBest, true));
		std::string sWeight = RoundToString((double)weight,0);
		std::string sOverviewCPID = bPoolMiningMode ? "POOL" : GlobalCPUMiningCPID.cpid;

		//9-6-2015 Add RSA fields to overview
		if ((double)weight > 100000000000000)
		{
				sWeight = sWeight.substr(0,13) + "E" + RoundToString((double)sWeight.length()-13,0);
		}
		status = "&nbsp;<br>Blocks: " + RoundToString((double)nBestHeight,0) + "; PoR Difficulty: "
			+ RoundToString(PORDiff,3) + "; Net Weight: " + RoundToString(GetPoSKernelPS2(),2)
			+ "<br>DPOR Weight: " +  sWeight + "; Status: " + msMiningErrors
			+ "<br>Magnitude: " + RoundToString(boincmagnitude,2) + "; Project: " + msMiningProject
			+ "<br>CPID: " +  sOverviewCPID + " " +  msMiningErrors2 + " "
			+ "<br>" + msMiningErrors5 + " " + msMiningErrors6 + " " + msMiningErrors7 + " " + msMiningErrors8 + " "
			+ "<br>" + msRSAOverview + "<br>&nbsp;";
		//The last line break is for Windows 8.1 Huge Toolbar
		msGlobalStatus = status;
		return status;
	}
	catch (std::exception& e)
	{
			msMiningErrors = "Error obtaining status.";

			printf("Error obtaining status\r\n");
			return "";
		}
		catch(...)
		{
			msMiningErrors = "Error obtaining status (08-18-2014).";
			return "";
		}

}



std::string AppCache(std::string key)
{

	StructCPIDCache setting = mvAppCache["cache"+key];
	if (!setting.initialized)
	{
		setting.initialized=true;
		setting.xml = "";
		mvAppCache.insert(map<string,StructCPIDCache>::value_type("cache"+key,setting));
	    mvAppCache["cache"+key]=setting;
	}
	return setting.xml;
}



bool Timer_Main(std::string timer_name, int max_ms)
{
	mvTimers[timer_name] = mvTimers[timer_name] + 1;
	if (mvTimers[timer_name] > max_ms)
	{
		mvTimers[timer_name]=0;
		return true;
	}
	return false;
}



void WriteAppCache(std::string key, std::string value)
{
	StructCPIDCache setting = mvAppCache["cache"+key];
	if (!setting.initialized)
	{
		setting.initialized=true;
		setting.xml = "";
		mvAppCache.insert(map<string,StructCPIDCache>::value_type("cache"+key,setting));
	    mvAppCache["cache"+key]=setting;
	}
	setting.xml = value;
	mvAppCache["cache"+key]=setting;
}



void RegisterWallet(CWallet* pwalletIn)
{
    {
        LOCK(cs_setpwalletRegistered);
        setpwalletRegistered.insert(pwalletIn);
    }
}

void UnregisterWallet(CWallet* pwalletIn)
{
    {
        LOCK(cs_setpwalletRegistered);
        setpwalletRegistered.erase(pwalletIn);
    }
}


MiningCPID GetInitializedGlobalCPUMiningCPID(std::string cpid)
{

	MiningCPID mc = GetMiningCPID();
	mc.initialized = true;
	mc.cpid=cpid;
	mc.projectname = cpid;
	mc.cpidv2=cpid;
	mc.cpidhash = "";
	mc.email = cpid;
	mc.boincruntimepublickey = cpid;
	mc.rac=0;
	mc.encboincpublickey = "";
	mc.enccpid = "";
	mc.NetworkRAC = 0;
	mc.Magnitude = 0;
    mc.clientversion = "";
	mc.RSAWeight = GetRSAWeightByCPID(cpid);
	mc.LastPaymentTime = nLastBlockSolved;
	mc.diffbytes = 0;
	mc.lastblockhash = "0";
	return mc;
}


MiningCPID GetNextProject(bool bForce)
{



	if (GlobalCPUMiningCPID.projectname.length() > 3   &&  GlobalCPUMiningCPID.projectname != "INVESTOR"  && GlobalCPUMiningCPID.Magnitude > 1)
	{
				if (!Timer_Main("globalcpuminingcpid",10))
				{
					//Prevent Thrashing
					return GlobalCPUMiningCPID;
				}
	}


	std::string sBoincKey = GetArgument("boinckey","");
    if (!sBoincKey.empty())
	{
		if (fDebug3 && LessVerbose(50)) printf("Using cached boinckey for project %s\r\n",GlobalCPUMiningCPID.projectname.c_str());
					msMiningProject = GlobalCPUMiningCPID.projectname;
					msMiningCPID = GlobalCPUMiningCPID.cpid;
					if (LessVerbose(5)) printf("BoincKey - Mining project %s     RAC(%f)  enc %s\r\n",	GlobalCPUMiningCPID.projectname.c_str(), GlobalCPUMiningCPID.rac, msENCboincpublickey.c_str());
					double ProjectRAC = GetNetworkAvgByProject(GlobalCPUMiningCPID.projectname);
					GlobalCPUMiningCPID.NetworkRAC = ProjectRAC;
					mdMiningNetworkRAC = GlobalCPUMiningCPID.NetworkRAC;
					GlobalCPUMiningCPID.Magnitude = CalculatedMagnitude(GetAdjustedTime(),false);
					if (fDebug3) printf("(boinckey) For CPID %s Verified Magnitude = %f",GlobalCPUMiningCPID.cpid.c_str(),GlobalCPUMiningCPID.Magnitude);
					msMiningErrors = (msMiningCPID == "INVESTOR" || msPrimaryCPID=="INVESTOR" || msMiningCPID.empty()) ? "Staking Interest" : "Mining";
					GlobalCPUMiningCPID.RSAWeight = GetRSAWeightByCPID(GlobalCPUMiningCPID.cpid);
					GlobalCPUMiningCPID.LastPaymentTime = GetLastPaymentTimeByCPID(GlobalCPUMiningCPID.cpid);
					return GlobalCPUMiningCPID;
	}

	
	msMiningProject = "";
	msMiningCPID = "";
	mdMiningRAC = 0;
	msENCboincpublickey = "";
	GlobalCPUMiningCPID = GetInitializedGlobalCPUMiningCPID("");

	std::string email = GetArgument("email", "NA");
	boost::to_lower(email);



	if ( (IsInitialBlockDownload() || !bCPIDsLoaded) && !bForce)
	{
			if (LessVerbose(100))		    printf("CPUMiner: Gridcoin is downloading blocks Or CPIDs are not yet loaded...");
			MilliSleep(1);
			return GlobalCPUMiningCPID;
	}


	try
	{

		if (mvCPIDs.size() < 1)
		{
			if (fDebug && LessVerbose(10)) printf("Gridcoin has no CPIDs...");
			//Let control reach the investor area
		}

		int iValidProjects=0;
		//Count valid projects:
		for(map<string,StructCPID>::iterator ii=mvCPIDs.begin(); ii!=mvCPIDs.end(); ++ii)
		{
				StructCPID structcpid = mvCPIDs[(*ii).first];
				if (		msPrimaryCPID == structcpid.cpid &&
					structcpid.initialized && structcpid.Iscpidvalid)			iValidProjects++;
		}

		// Find next available CPU project:
		int iDistributedProject = 0;
		int iRow = 0;

		if (email=="" || email=="NA") iValidProjects = 0;  //Let control reach investor area


		if (iValidProjects > 0)
		{
		for (int i = 0; i <= 4;i++)
		{
			iRow=0;
			iDistributedProject = (rand() % iValidProjects)+1;

			for(map<string,StructCPID>::iterator ii=mvCPIDs.begin(); ii!=mvCPIDs.end(); ++ii)
			{
				StructCPID structcpid = mvCPIDs[(*ii).first];

				if (structcpid.initialized)
				{
					if (msPrimaryCPID == structcpid.cpid &&
						structcpid.Iscpidvalid && structcpid.projectname.length() > 1)
					{
							iRow++;
							if (i==4 || iDistributedProject == iRow)
							{
								if (true)
								{
									GlobalCPUMiningCPID.enccpid = structcpid.boincpublickey;
									bool checkcpid = IsCPIDValid_Retired(structcpid.cpid,GlobalCPUMiningCPID.enccpid);
									if (!checkcpid)
									{
										printf("CPID invalid %s  1.  ",structcpid.cpid.c_str());
										continue;
									}

									if (checkcpid)
									{

										GlobalCPUMiningCPID.email = email;

										if (LessVerbose(1) || fDebug || fDebug3) printf("Ready to CPU Mine project %s with CPID %s, RAC(%f) \r\n",
											structcpid.projectname.c_str(),structcpid.cpid.c_str(),
											structcpid.rac);
										//Required for project to be mined in a block:
										GlobalCPUMiningCPID.cpid=structcpid.cpid;
										GlobalCPUMiningCPID.projectname = structcpid.projectname;
										GlobalCPUMiningCPID.rac=structcpid.rac;
										GlobalCPUMiningCPID.encboincpublickey = structcpid.boincpublickey;
										GlobalCPUMiningCPID.encaes = structcpid.boincpublickey;


										GlobalCPUMiningCPID.boincruntimepublickey = structcpid.cpidhash;
										printf("\r\n GNP: Setting bpk to %s\r\n",structcpid.cpidhash.c_str());

										uint256 pbh = 1;
										GlobalCPUMiningCPID.cpidv2 = ComputeCPIDv2(GlobalCPUMiningCPID.email,GlobalCPUMiningCPID.boincruntimepublickey, pbh);
										GlobalCPUMiningCPID.lastblockhash = "0";
										// Sign the block
										GlobalCPUMiningCPID.BoincSignature = SignBlockWithCPID(GlobalCPUMiningCPID.cpid,GlobalCPUMiningCPID.lastblockhash);
								
										if (!IsCPIDValidv2(GlobalCPUMiningCPID,1))
										{
											printf("CPID INVALID (GetNextProject) %s, %s  ",GlobalCPUMiningCPID.cpid.c_str(),GlobalCPUMiningCPID.cpidv2.c_str());
											continue;
										}


										//Only used for global status:
										msMiningProject = structcpid.projectname;
										msMiningCPID = structcpid.cpid;
										mdMiningRAC = structcpid.rac;

										double ProjectRAC = GetNetworkAvgByProject(GlobalCPUMiningCPID.projectname);
										GlobalCPUMiningCPID.NetworkRAC = ProjectRAC;
										mdMiningNetworkRAC = GlobalCPUMiningCPID.NetworkRAC;
										GlobalCPUMiningCPID.Magnitude = CalculatedMagnitude(GetAdjustedTime(),false);
										if (fDebug && LessVerbose(2)) printf("For CPID %s Verified Magnitude = %f",GlobalCPUMiningCPID.cpid.c_str(),GlobalCPUMiningCPID.Magnitude);
										//Reserved for GRC Speech Synthesis
										msMiningErrors = (msMiningCPID == "INVESTOR" || msPrimaryCPID=="INVESTOR" || msMiningCPID.empty() || msPrimaryCPID.empty()) ? "Staking Interest" : "Boinc Mining";
										GlobalCPUMiningCPID.RSAWeight = GetRSAWeightByCPID(GlobalCPUMiningCPID.cpid);
										GlobalCPUMiningCPID.LastPaymentTime = GetLastPaymentTimeByCPID(GlobalCPUMiningCPID.cpid);
										return GlobalCPUMiningCPID;
									}
								}
							}

					}

				}
			}

		}
		}

		msMiningErrors = (msPrimaryCPID == "INVESTOR") ? "" : "All BOINC projects exhausted.";
		msMiningProject = "INVESTOR";
		msMiningCPID = "INVESTOR";
		mdMiningRAC = 0;
		msENCboincpublickey = "";
		GlobalCPUMiningCPID = GetInitializedGlobalCPUMiningCPID("INVESTOR");
		mdMiningNetworkRAC = 0;
		if (fDebug10) printf("-Investor mode-");

	  	}
		catch (std::exception& e)
		{
			msMiningErrors = "Error obtaining next project.  Error 16172014.";

			printf("Error obtaining next project\r\n");
		}
		catch(...)
		{
			msMiningErrors = "Error obtaining next project.  Error 06172014.";
			printf("Error obtaining next project 2.\r\n");
		}
		return GlobalCPUMiningCPID;

}



// check whether the passed transaction is from us
bool static IsFromMe(CTransaction& tx)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        if (pwallet->IsFromMe(tx))
            return true;
    return false;
}

// get the wallet transaction with the given hash (if it exists)
bool static GetTransaction(const uint256& hashTx, CWalletTx& wtx)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        if (pwallet->GetTransaction(hashTx,wtx))
            return true;
    return false;
}

// erases transaction with the given hash from all wallets
void static EraseFromWallets(uint256 hash)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->EraseFromWallet(hash);
}

// make sure all wallets know about the given transaction, in the given block
void SyncWithWallets(const CTransaction& tx, const CBlock* pblock, bool fUpdate, bool fConnect)
{
    if (!fConnect)
    {
        // ppcoin: wallets need to refund inputs when disconnecting coinstake
        if (tx.IsCoinStake())
        {
            BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
                if (pwallet->IsFromMe(tx))
                    pwallet->DisableTransaction(tx);
        }
        return;
    }

    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->AddToWalletIfInvolvingMe(tx, pblock, fUpdate);
}

// notify wallets about a new best chain
void static SetBestChain(const CBlockLocator& loc)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->SetBestChain(loc);
}

// notify wallets about an updated transaction
void static UpdatedTransaction(const uint256& hashTx)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->UpdatedTransaction(hashTx);
}

// dump all wallets
void static PrintWallets(const CBlock& block)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->PrintWallet(block);
}

// notify wallets about an incoming inventory (for request counts)
void static Inventory(const uint256& hash)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->Inventory(hash);
}

// ask wallets to resend their transactions
void ResendWalletTransactions(bool fForce)
{
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
        pwallet->ResendWalletTransactions(fForce);
}


double CoinToDouble(double surrogate)
{
	//Converts satoshis to a human double amount
	double coin = (double)surrogate/(double)COIN;
	return coin;
}

double GetTotalBalance()
{
	double total = 0;
    BOOST_FOREACH(CWallet* pwallet, setpwalletRegistered)
	{
        total = total + pwallet->GetBalance();
		total = total + pwallet->GetStake();
	}
	return total/COIN;
}
//////////////////////////////////////////////////////////////////////////////
//
// mapOrphanTransactions
//

bool AddOrphanTx(const CTransaction& tx)
{
    uint256 hash = tx.GetHash();
    if (mapOrphanTransactions.count(hash))
        return false;

    // Ignore big transactions, to avoid a
    // send-big-orphans memory exhaustion attack. If a peer has a legitimate
    // large transaction with a missing parent then we assume
    // it will rebroadcast it later, after the parent transaction(s)
    // have been mined or received.
    // 10,000 orphans, each of which is at most 5,000 bytes big is
    // at most 500 megabytes of orphans:

    size_t nSize = tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);

    if (nSize > 5000)
    {
        printf("ignoring large orphan tx (size: %" PRIszu ", hash: %s)\n", nSize, hash.ToString().substr(0,10).c_str());
        return false;
    }

    mapOrphanTransactions[hash] = tx;
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
        mapOrphanTransactionsByPrev[txin.prevout.hash].insert(hash);

    printf("stored orphan tx %s (mapsz %" PRIszu ")\n", hash.ToString().substr(0,10).c_str(),   mapOrphanTransactions.size());
    return true;
}

void static EraseOrphanTx(uint256 hash)
{
    if (!mapOrphanTransactions.count(hash))
        return;
    const CTransaction& tx = mapOrphanTransactions[hash];
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {
        mapOrphanTransactionsByPrev[txin.prevout.hash].erase(hash);
        if (mapOrphanTransactionsByPrev[txin.prevout.hash].empty())
            mapOrphanTransactionsByPrev.erase(txin.prevout.hash);
    }
    mapOrphanTransactions.erase(hash);
}

unsigned int LimitOrphanTxSize(unsigned int nMaxOrphans)
{
    unsigned int nEvicted = 0;
    while (mapOrphanTransactions.size() > nMaxOrphans)
    {
        // Evict a random orphan:
        uint256 randomhash = GetRandHash();
        map<uint256, CTransaction>::iterator it = mapOrphanTransactions.lower_bound(randomhash);
        if (it == mapOrphanTransactions.end())
            it = mapOrphanTransactions.begin();
        EraseOrphanTx(it->first);
        ++nEvicted;
    }
    return nEvicted;
}



std::string DefaultWalletAddress()
{
	try
	{
		//Gridcoin - Find the default public GRC address (since a user may have many receiving addresses):
		if (sDefaultWalletAddress.length() > 0) return sDefaultWalletAddress;
		string strAccount;
		BOOST_FOREACH(const PAIRTYPE(CTxDestination, string)& item, pwalletMain->mapAddressBook)
		{
    		 const CBitcoinAddress& address = item.first;
			 const std::string& strName = item.second;
			 bool fMine = IsMine(*pwalletMain, address.Get());
			 if (fMine && strName == "Default") 
			 {
				 sDefaultWalletAddress=CBitcoinAddress(address).ToString();
				 return sDefaultWalletAddress;
			 }
		}


		//Cant Find

		BOOST_FOREACH(const PAIRTYPE(CTxDestination, string)& item, pwalletMain->mapAddressBook)
		{
    		 const CBitcoinAddress& address = item.first;
			 //const std::string& strName = item.second;
			 bool fMine = IsMine(*pwalletMain, address.Get());
			 if (fMine)
			 {
				 sDefaultWalletAddress=CBitcoinAddress(address).ToString();
				 return sDefaultWalletAddress;
			 }
		}
	}
	catch (std::exception& e)
	{
		return "ERROR";
	}
    return "NA";
}






//////////////////////////////////////////////////////////////////////////////
//
// CTransaction and CTxIndex
//

bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout, CTxIndex& txindexRet)
{
    SetNull();
    if (!txdb.ReadTxIndex(prevout.hash, txindexRet))
        return false;
    if (!ReadFromDisk(txindexRet.pos))
        return false;
    if (prevout.n >= vout.size())
    {
        SetNull();
        return false;
    }
    return true;
}

bool CTransaction::ReadFromDisk(CTxDB& txdb, COutPoint prevout)
{
    CTxIndex txindex;
    return ReadFromDisk(txdb, prevout, txindex);
}

bool CTransaction::ReadFromDisk(COutPoint prevout)
{
    CTxDB txdb("r");
    CTxIndex txindex;
    return ReadFromDisk(txdb, prevout, txindex);
}





bool IsStandardTx(const CTransaction& tx)
{
	std::string reason = "";
    if (tx.nVersion > CTransaction::CURRENT_VERSION)
        return false;

    // Treat non-final transactions as non-standard to prevent a specific type
    // of double-spend attack, as well as DoS attacks. (if the transaction
    // can't be mined, the attacker isn't expending resources broadcasting it)
    // Basically we don't want to propagate transactions that can't included in
    // the next block.
    //
    // However, IsFinalTx() is confusing... Without arguments, it uses
    // chainActive.Height() to evaluate nLockTime; when a block is accepted, chainActive.Height()
    // is set to the value of nHeight in the block. However, when IsFinalTx()
    // is called within CBlock::AcceptBlock(), the height of the block *being*
    // evaluated is what is used. Thus if we want to know if a transaction can
    // be part of the *next* block, we need to call IsFinalTx() with one more
    // than chainActive.Height().
    //
    // Timestamps on the other hand don't get any special treatment, because we
    // can't know what timestamp the next block will have, and there aren't
    // timestamp applications where it matters.
    if (!IsFinalTx(tx, nBestHeight + 1)) {
        return false;
    }
    // nTime has different purpose from nLockTime but can be used in similar attacks
    if (tx.nTime > FutureDrift(GetAdjustedTime(), nBestHeight + 1)) {
        return false;
    }

    // Extremely large transactions with lots of inputs can cost the network
    // almost as much to process as they cost the sender in fees, because
    // computing signature hashes is O(ninputs*txsize). Limiting transactions
    // to MAX_STANDARD_TX_SIZE mitigates CPU exhaustion attacks.
    unsigned int sz = tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);
    if (sz >= MAX_STANDARD_TX_SIZE)
        return false;

    BOOST_FOREACH(const CTxIn& txin, tx.vin)
    {

		// Biggest 'standard' txin is a 15-of-15 P2SH multisig with compressed
		// keys. (remember the 520 byte limit on redeemScript size) That works
		// out to a (15*(33+1))+3=513 byte redeemScript, 513+1+15*(73+1)=1624
        // bytes of scriptSig, which we round off to 1650 bytes for some minor
		// future-proofing. That's also enough to spend a 20-of-20
		// CHECKMULTISIG scriptPubKey, though such a scriptPubKey is not
		// considered standard)

        if (txin.scriptSig.size() > 1650)
            return false;
        if (!txin.scriptSig.IsPushOnly())
            return false;
        if (fEnforceCanonical && !txin.scriptSig.HasCanonicalPushes()) {
            return false;
        }
    }

    unsigned int nDataOut = 0;
    txnouttype whichType;
    BOOST_FOREACH(const CTxOut& txout, tx.vout) {
        if (!::IsStandard(txout.scriptPubKey, whichType))
            return false;
        if (whichType == TX_NULL_DATA)
            nDataOut++;
        if (txout.nValue == 0)
            return false;
        if (fEnforceCanonical && !txout.scriptPubKey.HasCanonicalPushes()) {
            return false;
        }
    }


	// not more than one data txout per non-data txout is permitted
    // only one data txout is permitted too
    if (nDataOut > 1 && nDataOut > tx.vout.size()/2)
	{
        reason = "multi-op-return";
        return false;
    }


    return true;
}

bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime)
{
    AssertLockHeld(cs_main);
    // Time based nLockTime implemented in 0.1.6
    if (tx.nLockTime == 0)
        return true;
    if (nBlockHeight == 0)
        nBlockHeight = nBestHeight;
    if (nBlockTime == 0)
        nBlockTime = GetAdjustedTime();
    if ((int64_t)tx.nLockTime < ((int64_t)tx.nLockTime < LOCKTIME_THRESHOLD ? (int64_t)nBlockHeight : nBlockTime))
        return true;
    BOOST_FOREACH(const CTxIn& txin, tx.vin)
        if (!txin.IsFinal())
            return false;
    return true;
}

//
// Check transaction inputs, and make sure any
// pay-to-script-hash transactions are evaluating IsStandard scripts
//
// Why bother? To avoid denial-of-service attacks; an attacker
// can submit a standard HASH... OP_EQUAL transaction,
// which will get accepted into blocks. The redemption
// script can be anything; an attacker could use a very
// expensive-to-check-upon-redemption script like:
//   DUP CHECKSIG DROP ... repeated 100 times... OP_1
//
bool CTransaction::AreInputsStandard(const MapPrevTx& mapInputs) const
{
    if (IsCoinBase())
        return true; // Coinbases don't use vin normally

    for (unsigned int i = 0; i < vin.size(); i++)
    {
        const CTxOut& prev = GetOutputFor(vin[i], mapInputs);

        vector<vector<unsigned char> > vSolutions;
        txnouttype whichType;
        // get the scriptPubKey corresponding to this input:
        const CScript& prevScript = prev.scriptPubKey;
        if (!Solver(prevScript, whichType, vSolutions))
            return false;
        int nArgsExpected = ScriptSigArgsExpected(whichType, vSolutions);
        if (nArgsExpected < 0)
            return false;

        // Transactions with extra stuff in their scriptSigs are
        // non-standard. Note that this EvalScript() call will
        // be quick, because if there are any operations
        // beside "push data" in the scriptSig the
        // IsStandard() call returns false
        vector<vector<unsigned char> > stack;
		if (!EvalScript(stack, vin[i].scriptSig, *this, i, 0))            return false;

        if (whichType == TX_SCRIPTHASH)
        {
            if (stack.empty())
                return false;
            CScript subscript(stack.back().begin(), stack.back().end());
            vector<vector<unsigned char> > vSolutions2;
            txnouttype whichType2;
            if (!Solver(subscript, whichType2, vSolutions2))
                return false;
            if (whichType2 == TX_SCRIPTHASH)
                return false;

            int tmpExpected;
            tmpExpected = ScriptSigArgsExpected(whichType2, vSolutions2);
            if (tmpExpected < 0)
                return false;
            nArgsExpected += tmpExpected;
        }

        if (stack.size() != (unsigned int)nArgsExpected)
            return false;
    }

    return true;
}

unsigned int CTransaction::GetLegacySigOpCount() const
{
    unsigned int nSigOps = 0;
    BOOST_FOREACH(const CTxIn& txin, vin)
    {
        nSigOps += txin.scriptSig.GetSigOpCount(false);
    }
    BOOST_FOREACH(const CTxOut& txout, vout)
    {
        nSigOps += txout.scriptPubKey.GetSigOpCount(false);
    }
    return nSigOps;
}


int CMerkleTx::SetMerkleBranch(const CBlock* pblock)
{
    AssertLockHeld(cs_main);

    CBlock blockTmp;
    if (pblock == NULL)
    {
        // Load the block this tx is in
        CTxIndex txindex;
        if (!CTxDB("r").ReadTxIndex(GetHash(), txindex))
            return 0;
        if (!blockTmp.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos))
            return 0;
        pblock = &blockTmp;
    }

    // Update the tx's hashBlock
    hashBlock = pblock->GetHash();

    // Locate the transaction
    for (nIndex = 0; nIndex < (int)pblock->vtx.size(); nIndex++)
        if (pblock->vtx[nIndex] == *(CTransaction*)this)
            break;
    if (nIndex == (int)pblock->vtx.size())
    {
        vMerkleBranch.clear();
        nIndex = -1;
        printf("ERROR: SetMerkleBranch() : couldn't find tx in block\n");
        return 0;
    }

    // Fill in merkle branch
    vMerkleBranch = pblock->GetMerkleBranch(nIndex);

    // Is the tx in a block that's in the main chain
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi == mapBlockIndex.end())
        return 0;
    CBlockIndex* pindex = (*mi).second;
    if (!pindex || !pindex->IsInMainChain())
        return 0;

    return pindexBest->nHeight - pindex->nHeight + 1;
}




bool CTransaction::CheckTransaction() const
{
    // Basic checks that don't depend on any context
    if (vin.empty())
        return DoS(10, error("CTransaction::CheckTransaction() : vin empty"));
    if (vout.empty())
        return DoS(10, error("CTransaction::CheckTransaction() : vout empty"));
    // Size limits
    if (::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
        return DoS(100, error("CTransaction::CheckTransaction() : size limits failed"));

    // Check for negative or overflow output values
    int64_t nValueOut = 0;
    for (unsigned int i = 0; i < vout.size(); i++)
    {
        const CTxOut& txout = vout[i];
        if (txout.IsEmpty() && !IsCoinBase() && !IsCoinStake())
            return DoS(100, error("CTransaction::CheckTransaction() : txout empty for user transaction"));
        if (txout.nValue < 0)
            return DoS(100, error("CTransaction::CheckTransaction() : txout.nValue negative"));
        if (txout.nValue > MAX_MONEY)
            return DoS(100, error("CTransaction::CheckTransaction() : txout.nValue too high"));
        nValueOut += txout.nValue;
        if (!MoneyRange(nValueOut))
            return DoS(100, error("CTransaction::CheckTransaction() : txout total out of range"));
    }

    // Check for duplicate inputs
    set<COutPoint> vInOutPoints;
    BOOST_FOREACH(const CTxIn& txin, vin)
    {
        if (vInOutPoints.count(txin.prevout))
            return false;
        vInOutPoints.insert(txin.prevout);
    }

    if (IsCoinBase())
    {
        if (vin[0].scriptSig.size() < 2 || vin[0].scriptSig.size() > 100)
            return DoS(100, error("CTransaction::CheckTransaction() : coinbase script size is invalid"));
    }
    else
    {
        BOOST_FOREACH(const CTxIn& txin, vin)
            if (txin.prevout.IsNull())
                return DoS(10, error("CTransaction::CheckTransaction() : prevout is null"));
    }

    return true;
}

int64_t CTransaction::GetMinFee(unsigned int nBlockSize, enum GetMinFee_mode mode, unsigned int nBytes) const
{
    // Base fee is either MIN_TX_FEE or MIN_RELAY_TX_FEE
    int64_t nBaseFee = (mode == GMF_RELAY) ? MIN_RELAY_TX_FEE : MIN_TX_FEE;

    unsigned int nNewBlockSize = nBlockSize + nBytes;
    int64_t nMinFee = (1 + (int64_t)nBytes / 1000) * nBaseFee;

    // To limit dust spam, require MIN_TX_FEE/MIN_RELAY_TX_FEE if any output is less than 0.01
    if (nMinFee < nBaseFee)
    {
        BOOST_FOREACH(const CTxOut& txout, vout)
            if (txout.nValue < CENT)
                nMinFee = nBaseFee;
    }

    // Raise the price as the block approaches full
    if (nBlockSize != 1 && nNewBlockSize >= MAX_BLOCK_SIZE_GEN/2)
    {
        if (nNewBlockSize >= MAX_BLOCK_SIZE_GEN)
            return MAX_MONEY;
        nMinFee *= MAX_BLOCK_SIZE_GEN / (MAX_BLOCK_SIZE_GEN - nNewBlockSize);
    }

    if (!MoneyRange(nMinFee))
        nMinFee = MAX_MONEY;
    return nMinFee;
}


bool AcceptToMemoryPool(CTxMemPool& pool, CTransaction &tx, bool* pfMissingInputs)
{
    AssertLockHeld(cs_main);
    if (pfMissingInputs)
        *pfMissingInputs = false;

    if (!tx.CheckTransaction())
        return error("AcceptToMemoryPool : CheckTransaction failed");

    // Coinbase is only valid in a block, not as a loose transaction
    if (tx.IsCoinBase())
        return tx.DoS(100, error("AcceptToMemoryPool : coinbase as individual tx"));

    // ppcoin: coinstake is also only valid in a block, not as a loose transaction
    if (tx.IsCoinStake())
        return tx.DoS(100, error("AcceptToMemoryPool : coinstake as individual tx"));

    // Rather not work on nonstandard transactions (unless -testnet)
    if (!fTestNet && !IsStandardTx(tx))
        return error("AcceptToMemoryPool : nonstandard transaction type");

    // is it already in the memory pool?
    uint256 hash = tx.GetHash();
    if (pool.exists(hash))
        return false;

    // Check for conflicts with in-memory transactions
    CTransaction* ptxOld = NULL;
    {
		LOCK(pool.cs); // protect pool.mapNextTx
		for (unsigned int i = 0; i < tx.vin.size(); i++)
		{
			COutPoint outpoint = tx.vin[i].prevout;
			if (pool.mapNextTx.count(outpoint))
			{
				// Disable replacement feature for now
				return false;

				// Allow replacing with a newer version of the same transaction
				if (i != 0)
					return false;
				ptxOld = pool.mapNextTx[outpoint].ptx;
				if (IsFinalTx(*ptxOld))
					return false;
				if (!tx.IsNewerThan(*ptxOld))
					return false;
				for (unsigned int i = 0; i < tx.vin.size(); i++)
				{
					COutPoint outpoint = tx.vin[i].prevout;
					if (!pool.mapNextTx.count(outpoint) || pool.mapNextTx[outpoint].ptx != ptxOld)
						return false;
				}
				break;
			}
		}
    }

    {
        CTxDB txdb("r");

        // do we already have it?
        if (txdb.ContainsTx(hash))
            return false;

        MapPrevTx mapInputs;
        map<uint256, CTxIndex> mapUnused;
        bool fInvalid = false;
        if (!tx.FetchInputs(txdb, mapUnused, false, false, mapInputs, fInvalid))
        {
            if (fInvalid)
                return error("AcceptToMemoryPool : FetchInputs found invalid tx %s", hash.ToString().substr(0,10).c_str());
            if (pfMissingInputs)
                *pfMissingInputs = true;
            return false;
        }

        // Check for non-standard pay-to-script-hash in inputs
        if (!tx.AreInputsStandard(mapInputs) && !fTestNet)
            return error("AcceptToMemoryPool : nonstandard transaction input");

        // Note: if you modify this code to accept non-standard transactions, then
        // you should add code here to check that the transaction does a
        // reasonable number of ECDSA signature verifications.

        int64_t nFees = tx.GetValueIn(mapInputs)-tx.GetValueOut();
        unsigned int nSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

        // Don't accept it if it can't get into a block
        int64_t txMinFee = tx.GetMinFee(1000, GMF_RELAY, nSize);
        if (nFees < txMinFee)
            return error("AcceptToMemoryPool : not enough fees %s, %" PRId64 " < %" PRId64,
                         hash.ToString().c_str(),
                         nFees, txMinFee);

        // Continuously rate-limit free transactions
        // This mitigates 'penny-flooding' -- sending thousands of free transactions just to
        // be annoying or make others' transactions take longer to confirm.
        if (nFees < MIN_RELAY_TX_FEE)
        {
            static CCriticalSection cs;
            static double dFreeCount;
            static int64_t nLastTime;
            int64_t nNow =  GetAdjustedTime();

            {
                LOCK(pool.cs);
                // Use an exponentially decaying ~10-minute window:
                dFreeCount *= pow(1.0 - 1.0/600.0, (double)(nNow - nLastTime));
                nLastTime = nNow;
                // -limitfreerelay unit is thousand-bytes-per-minute
                // At default rate it would take over a month to fill 1GB
                if (dFreeCount > GetArg("-limitfreerelay", 15)*10*1000 && !IsFromMe(tx))
                    return error("AcceptToMemoryPool : free transaction rejected by rate limiter");
                if (fDebug)
                    printf("Rate limit dFreeCount: %g => %g\n", dFreeCount, dFreeCount+nSize);
                dFreeCount += nSize;
            }
        }

        // Check against previous transactions
        // This is done last to help prevent CPU exhaustion denial-of-service attacks.
        if (!tx.ConnectInputs(txdb, mapInputs, mapUnused, CDiskTxPos(1,1,1), pindexBest, false, false))
        {
			// If this happens repeatedly, purge peers
			if (TimerMain("AcceptToMemoryPool", 20))
			{
				printf("\r\nAcceptToMemoryPool::CleaningInboundConnections\r\n");
				CleanInboundConnections(true);
			}	
			if (fDebug || true)
			{
				return error("AcceptToMemoryPool : Unable to Connect Inputs %s", hash.ToString().c_str());
			}
			else
			{
				return false;
			}
	    }
    }

    // Store transaction in memory
    {
        LOCK(pool.cs);
        if (ptxOld)
        {
            printf("AcceptToMemoryPool : replacing tx %s with new version\n", ptxOld->GetHash().ToString().c_str());
            pool.remove(*ptxOld);
        }
        pool.addUnchecked(hash, tx);
    }

    ///// are we sure this is ok when loading transactions or restoring block txes
    // If updated, erase old tx from wallet
    if (ptxOld)
        EraseFromWallets(ptxOld->GetHash());
	if (fDebug)     printf("AcceptToMemoryPool : accepted %s (poolsz %" PRIszu ")\n",           hash.ToString().c_str(),           pool.mapTx.size());
    return true;
}

bool CTxMemPool::addUnchecked(const uint256& hash, CTransaction &tx)
{
    // Add to memory pool without checking anything.  Don't call this directly,
    // call AcceptToMemoryPool to properly check the transaction first.
    {
        mapTx[hash] = tx;
        for (unsigned int i = 0; i < tx.vin.size(); i++)
            mapNextTx[tx.vin[i].prevout] = CInPoint(&mapTx[hash], i);
        nTransactionsUpdated++;
    }
    return true;
}


bool CTxMemPool::remove(const CTransaction &tx, bool fRecursive)
{
    // Remove transaction from memory pool
    {
        LOCK(cs);
        uint256 hash = tx.GetHash();
        if (mapTx.count(hash))
        {
            if (fRecursive) {
                for (unsigned int i = 0; i < tx.vout.size(); i++) {
                    std::map<COutPoint, CInPoint>::iterator it = mapNextTx.find(COutPoint(hash, i));
                    if (it != mapNextTx.end())
                        remove(*it->second.ptx, true);
                }
            }
            BOOST_FOREACH(const CTxIn& txin, tx.vin)
                mapNextTx.erase(txin.prevout);
            mapTx.erase(hash);
            nTransactionsUpdated++;
        }
    }
    return true;
}

bool CTxMemPool::removeConflicts(const CTransaction &tx)
{
    // Remove transactions which depend on inputs of tx, recursively
    LOCK(cs);
    BOOST_FOREACH(const CTxIn &txin, tx.vin) {
        std::map<COutPoint, CInPoint>::iterator it = mapNextTx.find(txin.prevout);
        if (it != mapNextTx.end()) {
            const CTransaction &txConflict = *it->second.ptx;
            if (txConflict != tx)
                remove(txConflict, true);
        }
    }
    return true;
}

void CTxMemPool::clear()
{
    LOCK(cs);
    mapTx.clear();
    mapNextTx.clear();
    ++nTransactionsUpdated;
}

void CTxMemPool::queryHashes(std::vector<uint256>& vtxid)
{
    vtxid.clear();

    LOCK(cs);
    vtxid.reserve(mapTx.size());
    for (map<uint256, CTransaction>::iterator mi = mapTx.begin(); mi != mapTx.end(); ++mi)
        vtxid.push_back((*mi).first);
}




int CMerkleTx::GetDepthInMainChainINTERNAL(CBlockIndex* &pindexRet) const
{
    if (hashBlock == 0 || nIndex == -1)
        return 0;
    AssertLockHeld(cs_main);

    // Find the block it claims to be in
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashBlock);
    if (mi == mapBlockIndex.end())
        return 0;
    CBlockIndex* pindex = (*mi).second;
    if (!pindex || !pindex->IsInMainChain())
        return 0;

    // Make sure the merkle branch connects to this block
    if (!fMerkleVerified)
    {
        if (CBlock::CheckMerkleBranch(GetHash(), vMerkleBranch, nIndex) != pindex->hashMerkleRoot)
            return 0;
        fMerkleVerified = true;
    }

    pindexRet = pindex;
    return pindexBest->nHeight - pindex->nHeight + 1;
}

int CMerkleTx::GetDepthInMainChain(CBlockIndex* &pindexRet) const
{
    AssertLockHeld(cs_main);
    int nResult = GetDepthInMainChainINTERNAL(pindexRet);
    if (nResult == 0 && !mempool.exists(GetHash()))
        return -1; // Not in chain, not in mempool

    return nResult;
}

int CMerkleTx::GetBlocksToMaturity() const
{
    if (!(IsCoinBase() || IsCoinStake()))
        return 0;
    return max(0, (nCoinbaseMaturity+10) - GetDepthInMainChain());
}


bool CMerkleTx::AcceptToMemoryPool()
{
    return ::AcceptToMemoryPool(mempool, *this, NULL);
}



bool CWalletTx::AcceptWalletTransaction(CTxDB& txdb)
{

    {
        // Add previous supporting transactions first
        BOOST_FOREACH(CMerkleTx& tx, vtxPrev)
        {
            if (!(tx.IsCoinBase() || tx.IsCoinStake()))
            {
                uint256 hash = tx.GetHash();
                if (!mempool.exists(hash) && !txdb.ContainsTx(hash))
                    tx.AcceptToMemoryPool();
            }
        }
        return AcceptToMemoryPool();
    }
    return false;
}

bool CWalletTx::AcceptWalletTransaction()
{
    CTxDB txdb("r");
    return AcceptWalletTransaction(txdb);
}

int CTxIndex::GetDepthInMainChain() const
{
    // Read block header
    CBlock block;
    if (!block.ReadFromDisk(pos.nFile, pos.nBlockPos, false))
        return 0;
    // Find the block in the index
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(block.GetHash());
    if (mi == mapBlockIndex.end())
        return 0;
    CBlockIndex* pindex = (*mi).second;
    if (!pindex || !pindex->IsInMainChain())
        return 0;
    return 1 + nBestHeight - pindex->nHeight;
}

// Return transaction in tx, and if it was found inside a block, its hash is placed in hashBlock
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock)
{
    {
        LOCK(cs_main);
        {
            if (mempool.lookup(hash, tx))
            {
                return true;
            }
        }
        CTxDB txdb("r");
        CTxIndex txindex;
        if (tx.ReadFromDisk(txdb, COutPoint(hash, 0), txindex))
        {
            CBlock block;
            if (block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
                hashBlock = block.GetHash();
            return true;
        }
    }
    return false;
}






//////////////////////////////////////////////////////////////////////////////
//
// CBlock and CBlockIndex
//

static CBlockIndex* pblockindexFBBHLast;
CBlockIndex* FindBlockByHeight(int nHeight)
{
    CBlockIndex *pblockindex;
    if (nHeight < nBestHeight / 2)
        pblockindex = pindexGenesisBlock;
    else
        pblockindex = pindexBest;
    if (pblockindexFBBHLast && abs(nHeight - pblockindex->nHeight) > abs(nHeight - pblockindexFBBHLast->nHeight))
        pblockindex = pblockindexFBBHLast;
    while (pblockindex->nHeight > nHeight)
        pblockindex = pblockindex->pprev;
    while (pblockindex->nHeight < nHeight)
        pblockindex = pblockindex->pnext;
    pblockindexFBBHLast = pblockindex;
    return pblockindex;
}


CBlockIndex* RPCFindBlockByHeight(int nHeight)
{
	//This keeps the threads separated and ensures pointers are in distinct locations
    CBlockIndex *RPCpblockindex;
    if (nHeight < nBestHeight / 2)
        RPCpblockindex = pindexGenesisBlock;
    else
        RPCpblockindex = pindexBest;
    while (RPCpblockindex->nHeight > nHeight)
	{
        RPCpblockindex = RPCpblockindex->pprev;
	}
    while (RPCpblockindex->nHeight < nHeight)
	{
        RPCpblockindex = RPCpblockindex->pnext;
	}
    return RPCpblockindex;
}

CBlockIndex* MainFindBlockByHeight(int nHeight)
{
    CBlockIndex *Mainpblockindex;
    if (nHeight < nBestHeight / 2)
        Mainpblockindex = pindexGenesisBlock;
    else
        Mainpblockindex = pindexBest;
    while (Mainpblockindex->nHeight > nHeight)
	{
        Mainpblockindex = Mainpblockindex->pprev;
	}
    while (Mainpblockindex->nHeight < nHeight)
	{
        Mainpblockindex = Mainpblockindex->pnext;
	}
    return Mainpblockindex;
}



bool CBlock::ReadFromDisk(const CBlockIndex* pindex, bool fReadTransactions)
{
    if (!fReadTransactions)
    {
        *this = pindex->GetBlockHeader();
        return true;
    }
    if (!ReadFromDisk(pindex->nFile, pindex->nBlockPos, fReadTransactions))
        return false;
    if (GetHash() != pindex->GetBlockHash())
        return error("CBlock::ReadFromDisk() : GetHash() doesn't match index");
    return true;
}

uint256 static GetOrphanRoot(const CBlock* pblock)
{
    // Work back to the first block in the orphan chain
    while (mapOrphanBlocks.count(pblock->hashPrevBlock))
        pblock = mapOrphanBlocks[pblock->hashPrevBlock];
    return pblock->GetHash();
}

// ppcoin: find block wanted by given orphan block
uint256 WantedByOrphan(const CBlock* pblockOrphan)
{
    // Work back to the first block in the orphan chain
    while (mapOrphanBlocks.count(pblockOrphan->hashPrevBlock))
        pblockOrphan = mapOrphanBlocks[pblockOrphan->hashPrevBlock];
    return pblockOrphan->hashPrevBlock;
}


static CBigNum GetProofOfStakeLimit(int nHeight)
{
    if (IsProtocolV2(nHeight))
        return bnProofOfStakeLimitV2;
    else
        return bnProofOfStakeLimit;
}


double CalculatedMagnitude(int64_t locktime,bool bUseLederstrumpf)
{
	// Get neural network magnitude:
	std::string cpid = "";
	if (GlobalCPUMiningCPID.initialized && !GlobalCPUMiningCPID.cpid.empty()) cpid = GlobalCPUMiningCPID.cpid;
	StructCPID stDPOR = GetInitializedStructCPID2(cpid,mvDPOR);
	return bUseLederstrumpf ? LederstrumpfMagnitude2(stDPOR.Magnitude,locktime) : stDPOR.Magnitude;
}

double CalculatedMagnitude2(std::string cpid, int64_t locktime,bool bUseLederstrumpf)
{
	// Get neural network magnitude:
	StructCPID stDPOR = GetInitializedStructCPID2(cpid,mvDPOR);
	return bUseLederstrumpf ? LederstrumpfMagnitude2(stDPOR.Magnitude,locktime) : stDPOR.Magnitude;
}



// miner's coin base reward
int64_t GetProofOfWorkReward(int64_t nFees, int64_t locktime, int64_t height)
{
	//NOTE: THIS REWARD IS ONLY USED IN THE POW PHASE (Block < 8000):
    int64_t nSubsidy = CalculatedMagnitude(locktime,true) * COIN;
    if (fDebug && GetBoolArg("-printcreation"))
        printf("GetProofOfWorkReward() : create=%s nSubsidy=%" PRId64 "\n", FormatMoney(nSubsidy).c_str(), nSubsidy);
	if (nSubsidy < (30*COIN)) nSubsidy=30*COIN;
	//Gridcoin Foundation Block:
	if (height==10)
	{
		nSubsidy = nGenesisSupply * COIN;
	}
	if (fTestNet) nSubsidy += 1000*COIN;

    return nSubsidy + nFees;
}


int64_t GetProofOfWorkMaxReward(int64_t nFees, int64_t locktime, int64_t height)
{
	int64_t nSubsidy = (GetMaximumBoincSubsidy(locktime)+1) * COIN;
	if (height==10)
	{
		//R.Halford: 10-11-2014: Gridcoin Foundation Block:
		//Note: Gridcoin Classic emitted these coins.  So we had to add them to block 10.  The coins were burned then given back to the owners that mined them in classic (as research coins).
		nSubsidy = nGenesisSupply * COIN;
	}

	if (fTestNet) nSubsidy += 1000*COIN;
    return nSubsidy + nFees;
}

//Survey Results: Start inflation rate: 9%, end=1%, 30 day steps, 9 steps, mag multiplier start: 2, mag end .3, 9 steps
int64_t GetMaximumBoincSubsidy(int64_t nTime)
{
	// Gridcoin Global Daily Maximum Researcher Subsidy Schedule
	int MaxSubsidy = 500;
    if (nTime >= 1410393600 && nTime <= 1417305600) MaxSubsidy = 	500; // between inception  and 11-30-2014
	if (nTime >= 1417305600 && nTime <= 1419897600) MaxSubsidy = 	400; // between 11-30-2014 and 12-30-2014
	if (nTime >= 1419897600 && nTime <= 1422576000) MaxSubsidy = 	400; // between 12-30-2014 and 01-30-2015
	if (nTime >= 1422576000 && nTime <= 1425254400) MaxSubsidy = 	300; // between 01-30-2015 and 02-28-2015
	if (nTime >= 1425254400 && nTime <= 1427673600) MaxSubsidy = 	250; // between 02-28-2015 and 03-30-2015
	if (nTime >= 1427673600 && nTime <= 1430352000) MaxSubsidy = 	200; // between 03-30-2015 and 04-30-2015
	if (nTime >= 1430352000 && nTime <= 1438310876) MaxSubsidy = 	150; // between 05-01-2015 and 07-31-2015
	if (nTime >= 1438310876 && nTime <= 1445309276) MaxSubsidy = 	100; // between 08-01-2015 and 10-20-2015
	if (nTime >= 1445309276 && nTime <= 1447977700) MaxSubsidy = 	 75; // between 10-20-2015 and 11-20-2015
	if (nTime > 1447977700)                         MaxSubsidy =   	 50; // from  11-20-2015 forever
    return MaxSubsidy+.5;  //The .5 allows for fractional amounts after the 4th decimal place (used to store the POR indicator)
}

int64_t GetCoinYearReward(int64_t nTime)
{
	// Gridcoin Global Interest Rate Schedule
	int64_t INTEREST = 9;
   	if (nTime >= 1410393600 && nTime <= 1417305600) INTEREST = 	 9 * CENT; // 09% between inception  and 11-30-2014
	if (nTime >= 1417305600 && nTime <= 1419897600) INTEREST = 	 8 * CENT; // 08% between 11-30-2014 and 12-30-2014
	if (nTime >= 1419897600 && nTime <= 1422576000) INTEREST = 	 8 * CENT; // 08% between 12-30-2014 and 01-30-2015
	if (nTime >= 1422576000 && nTime <= 1425254400) INTEREST = 	 7 * CENT; // 07% between 01-30-2015 and 02-30-2015
	if (nTime >= 1425254400 && nTime <= 1427673600) INTEREST = 	 6 * CENT; // 06% between 02-30-2015 and 03-30-2015
	if (nTime >= 1427673600 && nTime <= 1430352000) INTEREST = 	 5 * CENT; // 05% between 03-30-2015 and 04-30-2015
	if (nTime >= 1430352000 && nTime <= 1438310876) INTEREST =   4 * CENT; // 04% between 05-01-2015 and 07-31-2015
	if (nTime >= 1438310876 && nTime <= 1447977700) INTEREST =   3 * CENT; // 03% between 08-01-2015 and 11-20-2015
	if (nTime > 1447977700)                         INTEREST = 1.5 * CENT; //1.5% from 11-21-2015 forever
	return INTEREST;
}

double GetMagnitudeMultiplier(int64_t nTime)
{
	// Gridcoin Global Resarch Subsidy Multiplier Schedule
	double magnitude_multiplier = 2;
	if (nTime >= 1410393600 && nTime <= 1417305600) magnitude_multiplier =    2;  // between inception and 11-30-2014
	if (nTime >= 1417305600 && nTime <= 1419897600) magnitude_multiplier =  1.5;  // between 11-30-2014 and 12-30-2014
	if (nTime >= 1419897600 && nTime <= 1422576000) magnitude_multiplier =  1.5;  // between 12-30-2014 and 01-30-2015
	if (nTime >= 1422576000 && nTime <= 1425254400) magnitude_multiplier =    1;  // between 01-30-2015 and 02-30-2015
	if (nTime >= 1425254400 && nTime <= 1427673600) magnitude_multiplier =   .9;  // between 02-30-2015 and 03-30-2015
	if (nTime >= 1427673600 && nTime <= 1430352000) magnitude_multiplier =   .8;  // between 03-30-2015 and 04-30-2015
    if (nTime >= 1430352000 && nTime <= 1438310876) magnitude_multiplier =   .7;  // between 05-01-2015 and 07-31-2015
	if (nTime >= 1438310876 && nTime <= 1447977700) magnitude_multiplier =  .60;  // between 08-01-2015 and 11-20-2015
	if (nTime > 1447977700)                         magnitude_multiplier =  .50;  // from 11-21-2015  forever
	return magnitude_multiplier;
}


int64_t GetProofOfStakeMaxReward(int64_t nCoinAge, int64_t nFees, int64_t locktime)
{
	int64_t nInterest = nCoinAge * GetCoinYearReward(locktime) * 33 / (365 * 33 + 8);
	nInterest += 10*COIN;
	int64_t nBoinc    = (GetMaximumBoincSubsidy(locktime)+1) * COIN;
	int64_t nSubsidy  = nInterest + nBoinc;
    return nSubsidy + nFees;
}

double GetProofOfResearchReward(std::string cpid, bool VerifyingBlock)
{

		StructCPID mag = GetInitializedStructCPID2(cpid,mvMagnitudes);

		if (!mag.initialized) return 0;
		double owed = (mag.owed*1.0);
		if (owed < 0) owed = 0;
		// Coarse Payment Rule (helps prevent sync problems):
		if (!VerifyingBlock)
		{
			//If owed less than 4% of max subsidy, assess at 0:
			if (owed < (GetMaximumBoincSubsidy(GetAdjustedTime())/50))
			{
				owed = 0;
			}
			//Coarse payment rule:
			if (mag.totalowed > (GetMaximumBoincSubsidy(GetAdjustedTime())*2))
			{
				//If owed more than 2* Max Block, pay normal amount
	            owed = (owed*1);
			}
			else
			{
				owed = owed/2;
			}

			if (owed > (GetMaximumBoincSubsidy(GetAdjustedTime()))) owed = GetMaximumBoincSubsidy(GetAdjustedTime());


		}
		//End of Coarse Payment Rule
		return owed * COIN;
}


// miner's coin stake reward based on coin age spent (coin-days)

int64_t GetProofOfStakeReward(int64_t nCoinAge, int64_t nFees, std::string cpid,
	bool VerifyingBlock, int VerificationPhase, int64_t nTime, CBlockIndex* pindexLast, std::string operation,
	double& OUT_POR, double& OUT_INTEREST, double& dAccrualAge, double& dMagnitudeUnit, double& AvgMagnitude)
{

	// Non Research Age - RSA Mode - Legacy (before 10-20-2015)
	if (!IsResearchAgeEnabled(pindexLast->nHeight))
	{
			int64_t nInterest = nCoinAge * GetCoinYearReward(nTime) * 33 / (365 * 33 + 8);
			int64_t nBoinc    = GetProofOfResearchReward(cpid,VerifyingBlock);
			int64_t nSubsidy  = nInterest + nBoinc;
			if (fDebug10 || GetBoolArg("-printcreation"))
			{
				printf("GetProofOfStakeReward(): create=%s nCoinAge=%" PRId64 " nBoinc=%" PRId64 "   \n",
				FormatMoney(nSubsidy).c_str(), nCoinAge, nBoinc);
			}
			int64_t maxStakeReward1 = GetProofOfStakeMaxReward(nCoinAge, nFees, nTime);
			int64_t maxStakeReward2 = GetProofOfStakeMaxReward(nCoinAge, nFees, GetAdjustedTime());
			int64_t maxStakeReward = Floor(maxStakeReward1,maxStakeReward2);
			if ((nSubsidy+nFees) > maxStakeReward) nSubsidy = maxStakeReward-nFees;
			int64_t nTotalSubsidy = nSubsidy + nFees;
			if (nBoinc > 1)
			{
				std::string sTotalSubsidy = RoundToString(CoinToDouble(nTotalSubsidy)+.00000123,8);
				if (sTotalSubsidy.length() > 7)
				{
					sTotalSubsidy = sTotalSubsidy.substr(0,sTotalSubsidy.length()-4) + "0124";
					nTotalSubsidy = cdbl(sTotalSubsidy,8)*COIN;
				}
			}

			OUT_POR = CoinToDouble(nBoinc);
			OUT_INTEREST = CoinToDouble(nInterest);
			return nTotalSubsidy;
	}
	else
	{
			// Research Age Subsidy - PROD
		    int64_t nBoinc = ComputeResearchAccrual(nTime, cpid, operation, pindexLast, VerifyingBlock, VerificationPhase, dAccrualAge, dMagnitudeUnit, AvgMagnitude);
			int64_t nInterest = nCoinAge * GetCoinYearReward(nTime) * 33 / (365 * 33 + 8);

			// TestNet: For any subsidy < 30 day duration, ensure 100% that we have a start magnitude and an end magnitude, otherwise make subsidy 0 : PASS
			// TestNet: For any subsidy > 30 day duration, ensure 100% that we have a midpoint magnitude in Every Period, otherwise, make subsidy 0 : In Test as of 09-06-2015
			// TestNet: Ensure no magnitudes are out of bounds to ensure we do not generate an insane payment : PASS (Lifetime PPD takes care of this)
			// TestNet: Any subsidy with a duration wider than 6 months should not be paid : PASS

			int64_t maxStakeReward = GetMaximumBoincSubsidy(nTime) * COIN * 255;

			if (nBoinc > maxStakeReward) nBoinc = maxStakeReward;
			int64_t nSubsidy = nInterest + nBoinc;

			if (fDebug10 || GetBoolArg("-printcreation"))
			{
				printf("GetProofOfStakeReward(): create=%s nCoinAge=%" PRId64 " nBoinc=%" PRId64 "   \n",
				FormatMoney(nSubsidy).c_str(), nCoinAge, nBoinc);
			}

			int64_t nTotalSubsidy = nSubsidy + nFees;
			if (nBoinc > 1)
			{
				std::string sTotalSubsidy = RoundToString(CoinToDouble(nTotalSubsidy)+.00000123,8);
				if (sTotalSubsidy.length() > 7)
				{
					sTotalSubsidy = sTotalSubsidy.substr(0,sTotalSubsidy.length()-4) + "0124";
					nTotalSubsidy = cdbl(sTotalSubsidy,8)*COIN;
				}
			}

			OUT_POR = CoinToDouble(nBoinc);
			OUT_INTEREST = CoinToDouble(nInterest);
			return nTotalSubsidy;

	}
}



static const int64_t nTargetTimespan = 16 * 60;  // 16 mins

//
// maximum nBits value could possible be required nTime after
//
unsigned int ComputeMaxBits(CBigNum bnTargetLimit, unsigned int nBase, int64_t nTime)
{
    CBigNum bnResult;
    bnResult.SetCompact(nBase);
    bnResult *= 2;
    while (nTime > 0 && bnResult < bnTargetLimit)
    {
        // Maximum 200% adjustment per day...
        bnResult *= 2;
        nTime -= 24 * 60 * 60;
    }
    if (bnResult > bnTargetLimit)
        bnResult = bnTargetLimit;
    return bnResult.GetCompact();
}

//
// minimum amount of work that could possibly be required nTime after
// minimum proof-of-work required was nBase
//
unsigned int ComputeMinWork(unsigned int nBase, int64_t nTime)
{
    return ComputeMaxBits(bnProofOfWorkLimit, nBase, nTime);
}

//
// minimum amount of stake that could possibly be required nTime after
// minimum proof-of-stake required was nBase
//
unsigned int ComputeMinStake(unsigned int nBase, int64_t nTime, unsigned int nBlockTime)
{
    return ComputeMaxBits(bnProofOfStakeLimit, nBase, nTime);
}


// ppcoin: find last block index up to pindex
const CBlockIndex* GetLastBlockIndex(const CBlockIndex* pindex, bool fProofOfStake)
{
    while (pindex && pindex->pprev && (pindex->IsProofOfStake() != fProofOfStake))
        pindex = pindex->pprev;
    return pindex;
}


static unsigned int GetNextTargetRequiredV1(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    CBigNum bnTargetLimit = fProofOfStake ? bnProofOfStakeLimit : bnProofOfWorkLimit;

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nTargetSpacing = GetTargetSpacing(pindexLast->nHeight);
    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew > bnTargetLimit)
        bnNew = bnTargetLimit;

    return bnNew.GetCompact();
}

static unsigned int GetNextTargetRequiredV2(const CBlockIndex* pindexLast, bool fProofOfStake)
{
    CBigNum bnTargetLimit = fProofOfStake ? GetProofOfStakeLimit(pindexLast->nHeight) : bnProofOfWorkLimit;

    if (pindexLast == NULL)
        return bnTargetLimit.GetCompact(); // genesis block

    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexLast, fProofOfStake);
    if (pindexPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // first block
    const CBlockIndex* pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, fProofOfStake);
    if (pindexPrevPrev->pprev == NULL)
        return bnTargetLimit.GetCompact(); // second block

    int64_t nTargetSpacing = GetTargetSpacing(pindexLast->nHeight);
    int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    if (nActualSpacing < 0)
        nActualSpacing = nTargetSpacing;

    // ppcoin: target change every block
    // ppcoin: retarget with exponential moving toward target spacing
    CBigNum bnNew;
    bnNew.SetCompact(pindexPrev->nBits);

	//Gridcoin - Reset Diff to 1 on 12-19-2014 (R Halford) - Diff sticking at 2065 due to many incompatible features
	if (pindexLast->nHeight >= 91387 && pindexLast->nHeight <= 91500)
	{
		    return bnTargetLimit.GetCompact();
	}

	//1-14-2015 R Halford - Make diff reset to zero after periods of exploding diff:
	double PORDiff = GetDifficulty(GetLastBlockIndex(pindexBest, true));
	if (PORDiff > 900000)
	{
		    return bnTargetLimit.GetCompact();
	}


	//Since our nTargetTimespan is (16 * 60) or 16 mins and our TargetSpacing = 64, the nInterval = 15 min

    int64_t nInterval = nTargetTimespan / nTargetSpacing;
    bnNew *= ((nInterval - 1) * nTargetSpacing + nActualSpacing + nActualSpacing);
    bnNew /= ((nInterval + 1) * nTargetSpacing);

    if (bnNew <= 0 || bnNew > bnTargetLimit)
	{
	    bnNew = bnTargetLimit;
	}

    return bnNew.GetCompact();
}

unsigned int GetNextTargetRequired(const CBlockIndex* pindexLast, bool fProofOfStake)
{
	//After block 89600, new diff algorithm is used
    if (pindexLast->nHeight < 89600)
        return GetNextTargetRequiredV1(pindexLast, fProofOfStake);
    else
        return GetNextTargetRequiredV2(pindexLast, fProofOfStake);
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    CBigNum bnTarget;
    bnTarget.SetCompact(nBits);

    // Check range
    if (bnTarget <= 0 || bnTarget > bnProofOfWorkLimit)
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget.getuint256())
        return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}

// Return maximum amount of blocks that other nodes claim to have
int GetNumBlocksOfPeers()
{
	if (IsLockTimeWithinMinutes(nLastCalculatedMedianPeerCount,1))
	{
		return nLastMedianPeerCount;
	}
	nLastCalculatedMedianPeerCount = GetAdjustedTime();
	nLastMedianPeerCount = std::max(cPeerBlockCounts.median(), Checkpoints::GetTotalBlocksEstimate());
    return nLastMedianPeerCount;
}

bool IsInitialBlockDownload()
{
    LOCK(cs_main);
    if (pindexBest == NULL || nBestHeight < GetNumBlocksOfPeers())
        return true;
    static int64_t nLastUpdate;
    static CBlockIndex* pindexLastBest;
    if (pindexBest != pindexLastBest)
    {
        pindexLastBest = pindexBest;
        nLastUpdate =  GetAdjustedTime();
    }
    return ( GetAdjustedTime() - nLastUpdate < 15 &&
            pindexBest->GetBlockTime() <  GetAdjustedTime() - 8 * 60 * 60);
}

void static InvalidChainFound(CBlockIndex* pindexNew)
{
    if (pindexNew->nChainTrust > nBestInvalidTrust)
    {
        nBestInvalidTrust = pindexNew->nChainTrust;
        CTxDB().WriteBestInvalidTrust(CBigNum(nBestInvalidTrust));
        uiInterface.NotifyBlocksChanged();
    }

    uint256 nBestInvalidBlockTrust = pindexNew->nChainTrust - pindexNew->pprev->nChainTrust;
    uint256 nBestBlockTrust = pindexBest->nHeight != 0 ? (pindexBest->nChainTrust - pindexBest->pprev->nChainTrust) : pindexBest->nChainTrust;

    printf("InvalidChainFound: invalid block=%s  height=%d  trust=%s  blocktrust=%" PRId64 "  date=%s\n",
      pindexNew->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->nHeight,
      CBigNum(pindexNew->nChainTrust).ToString().c_str(), nBestInvalidBlockTrust.Get64(),
      DateTimeStrFormat("%x %H:%M:%S", pindexNew->GetBlockTime()).c_str());
    printf("InvalidChainFound:  current best=%s  height=%d  trust=%s  blocktrust=%" PRId64 "  date=%s\n",
      hashBestChain.ToString().substr(0,20).c_str(), nBestHeight,
      CBigNum(pindexBest->nChainTrust).ToString().c_str(),
      nBestBlockTrust.Get64(),
      DateTimeStrFormat("%x %H:%M:%S", pindexBest->GetBlockTime()).c_str());
}


void CBlock::UpdateTime(const CBlockIndex* pindexPrev)
{
    nTime = max(GetBlockTime(), GetAdjustedTime());
}



bool CTransaction::DisconnectInputs(CTxDB& txdb)
{
    // Relinquish previous transactions' spent pointers
    if (!IsCoinBase())
    {
        BOOST_FOREACH(const CTxIn& txin, vin)
        {
            COutPoint prevout = txin.prevout;
            // Get prev txindex from disk
            CTxIndex txindex;
            if (!txdb.ReadTxIndex(prevout.hash, txindex))
                return error("DisconnectInputs() : ReadTxIndex failed");

            if (prevout.n >= txindex.vSpent.size())
                return error("DisconnectInputs() : prevout.n out of range");

            // Mark outpoint as not spent
            txindex.vSpent[prevout.n].SetNull();

            // Write back
            if (!txdb.UpdateTxIndex(prevout.hash, txindex))
                return error("DisconnectInputs() : UpdateTxIndex failed");
        }
    }

    // Remove transaction from index
    // This can fail if a duplicate of this transaction was in a chain that got
    // reorganized away. This is only possible if this transaction was completely
    // spent, so erasing it would be a no-op anyway.
    txdb.EraseTxIndex(*this);

    return true;
}


bool CTransaction::FetchInputs(CTxDB& txdb, const map<uint256, CTxIndex>& mapTestPool,
                               bool fBlock, bool fMiner, MapPrevTx& inputsRet, bool& fInvalid)
{
    // FetchInputs can return false either because we just haven't seen some inputs
    // (in which case the transaction should be stored as an orphan)
    // or because the transaction is malformed (in which case the transaction should
    // be dropped).  If tx is definitely invalid, fInvalid will be set to true.
    fInvalid = false;

    if (IsCoinBase())
        return true; // Coinbase transactions have no inputs to fetch.

    for (unsigned int i = 0; i < vin.size(); i++)
    {
        COutPoint prevout = vin[i].prevout;
        if (inputsRet.count(prevout.hash))
            continue; // Got it already

        // Read txindex
        CTxIndex& txindex = inputsRet[prevout.hash].first;
        bool fFound = true;
        if ((fBlock || fMiner) && mapTestPool.count(prevout.hash))
        {
            // Get txindex from current proposed changes
            txindex = mapTestPool.find(prevout.hash)->second;
        }
        else
        {
            // Read txindex from txdb
            fFound = txdb.ReadTxIndex(prevout.hash, txindex);
        }
        if (!fFound && (fBlock || fMiner))
            return fMiner ? false : error("FetchInputs() : %s prev tx %s index entry not found", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());

        // Read txPrev
        CTransaction& txPrev = inputsRet[prevout.hash].second;
        if (!fFound || txindex.pos == CDiskTxPos(1,1,1))
        {
            // Get prev tx from single transactions in memory
            if (!mempool.lookup(prevout.hash, txPrev))
			{
				if (fDebug) printf("FetchInputs() : %s mempool Tx prev not found %s", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());
				return false;
			}
            if (!fFound)
                txindex.vSpent.resize(txPrev.vout.size());
        }
        else
        {
            // Get prev tx from disk
            if (!txPrev.ReadFromDisk(txindex.pos))
                return error("FetchInputs() : %s ReadFromDisk prev tx %s failed", GetHash().ToString().substr(0,10).c_str(),  prevout.hash.ToString().substr(0,10).c_str());
        }
    }

    // Make sure all prevout.n indexes are valid:
    for (unsigned int i = 0; i < vin.size(); i++)
    {
        const COutPoint prevout = vin[i].prevout;
        assert(inputsRet.count(prevout.hash) != 0);
        const CTxIndex& txindex = inputsRet[prevout.hash].first;
        const CTransaction& txPrev = inputsRet[prevout.hash].second;
        if (prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
        {
            // Revisit this if/when transaction replacement is implemented and allows
            // adding inputs:
            fInvalid = true;
            return DoS(100, error("FetchInputs() : %s prevout.n out of range %d %" PRIszu " %" PRIszu " prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));
        }
    }

    return true;
}

const CTxOut& CTransaction::GetOutputFor(const CTxIn& input, const MapPrevTx& inputs) const
{
    MapPrevTx::const_iterator mi = inputs.find(input.prevout.hash);
    if (mi == inputs.end())
        throw std::runtime_error("CTransaction::GetOutputFor() : prevout.hash not found");

    const CTransaction& txPrev = (mi->second).second;
    if (input.prevout.n >= txPrev.vout.size())
        throw std::runtime_error("CTransaction::GetOutputFor() : prevout.n out of range");

    return txPrev.vout[input.prevout.n];
}





void WriteStringToFile(const boost::filesystem::path &path, std::string sOut)
{
    FILE* file = fopen(path.string().c_str(), "w");
    if (file)
    {
        fprintf(file, "%s\r\n", sOut.c_str());
        fclose(file);
    }
}




std::vector<std::string> &split_bychar(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split_bychar(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    split_bychar(s, delim, elems);
    return elems;
}


std::vector<std::string> split(std::string s, std::string delim)
{
	//Split a std::string by a std::string delimiter into a vector of strings:
	size_t pos = 0;
	std::string token;
	std::vector<std::string> elems;
	while ((pos = s.find(delim)) != std::string::npos)
	{
		token = s.substr(0, pos);
		elems.push_back(token);
		s.erase(0, pos + delim.length());
	}
	elems.push_back(s);
	return elems;

}



int64_t CTransaction::GetValueIn(const MapPrevTx& inputs) const
{
    if (IsCoinBase())
        return 0;

    int64_t nResult = 0;
    for (unsigned int i = 0; i < vin.size(); i++)
    {
        nResult += GetOutputFor(vin[i], inputs).nValue;
    }
    return nResult;

}


double PreviousBlockAge()
{
    	if (nBestHeight < 10) return 99999;
		if (IsLockTimeWithinMinutes(nLastCalculatedMedianTimePast,1))
		{
			return nLastBlockAge;
		}
		nLastCalculatedMedianTimePast = GetAdjustedTime();
	    // Returns the time in seconds since the last block:
		double nTime = max(pindexBest->GetMedianTimePast()+1, GetAdjustedTime());
		double nActualTimespan = nTime - pindexBest->pprev->GetBlockTime();
		nLastBlockAge = nActualTimespan;
		return nActualTimespan;
}



bool ClientOutOfSync()
{
	//This function will return True if the client is downloading blocks, reindexing, or out of sync by more than 30 blocks as compared to its peers, or if its best block is over 30 mins old
	double lastblockage = PreviousBlockAge();
	if (lastblockage > (30*60)) return true;
	if (fReindex || fImporting ) return true;
	if (pindexBest == NULL || nBestHeight < GetNumBlocksOfPeers()-30) return true;
	return false;
}



bool OutOfSyncByMoreThan(double dMinutes)
{
	double lastblockage = PreviousBlockAge();
	if (lastblockage > (60*dMinutes)) return true;
	if (fReindex || fImporting ) return true;
	if (pindexBest == NULL || nBestHeight < GetNumBlocksOfPeers()-30) return true;
	return false;
}



bool OutOfSyncByAge()
{
	double lastblockage = PreviousBlockAge();
	if (lastblockage > (60*30)) return true;
	if (fReindex || fImporting ) return true;
	return false;
}


bool LessVerbose(int iMax1000)
{
	 //Returns True when RND() level is lower than the number presented
	 int iVerbosityLevel = rand() % 1000;
	 if (iVerbosityLevel < iMax1000) return true;
	 return false;
}


bool KeyEnabled(std::string key)
{
	if (mapArgs.count("-" + key))
	{
			std::string sBool = GetArg("-" + key, "false");
			if (sBool == "true") return true;
	}
	return false;
}


bool OutOfSyncByAgeWithChanceOfMining()
{
	// If the client is out of sync, we dont want it to mine orphan blocks on its own fork, so we return OOS when that is the case 95% of the time:
	// If the client is in sync, this function returns false and the client mines.
	// The reason we allow mining 5% of the time, is if all nodes leave Gridcoin, we want someone to be able to jump start the coin in that extremely rare circumstance (IE End of Life, or Network Outage across the country, etc).
	try
	{
		    if (fTestNet) return false;
		    if (KeyEnabled("overrideoutofsyncrule")) return false;
			bool oosbyage = OutOfSyncByAge();
			//Rule 1: If  Last Block Out of sync by Age - Return Out of Sync 95% of the time:
			if (oosbyage) if (LessVerbose(900)) return true;
			// Rule 2 : Dont mine on Fork Rule:
	     	//If the diff is < .00015 in Prod, Most likely the client is mining on a fork: (Make it exceedingly hard):
			double PORDiff = GetDifficulty(GetLastBlockIndex(pindexBest, true));
			if (!fTestNet && PORDiff < .00010)
			{
				printf("Most likely you are mining on a fork! Diff %f",PORDiff);
				if (LessVerbose(950)) return true;
			}
			return false;
	}
	catch (std::exception &e)
	{
				printf("Error while assessing Sync Condition\r\n");
				return true;
	}
	catch(...)
	{
				printf("Error while assessing Sync Condition[2].\r\n");
				return true;
	}
	return true;

}


int Races(int iMax1000)
{
	 int i = rand() % iMax1000;
	 if (i < 1) i = 1;
	 return i;
}


unsigned int CTransaction::GetP2SHSigOpCount(const MapPrevTx& inputs) const
{
    if (IsCoinBase())
        return 0;

    unsigned int nSigOps = 0;
    for (unsigned int i = 0; i < vin.size(); i++)
    {
        const CTxOut& prevout = GetOutputFor(vin[i], inputs);
        if (prevout.scriptPubKey.IsPayToScriptHash())
            nSigOps += prevout.scriptPubKey.GetSigOpCount(vin[i].scriptSig);
    }
    return nSigOps;
}

bool CTransaction::ConnectInputs(CTxDB& txdb, MapPrevTx inputs, map<uint256, CTxIndex>& mapTestPool, const CDiskTxPos& posThisTx,
    const CBlockIndex* pindexBlock, bool fBlock, bool fMiner)
{
    // Take over previous transactions' spent pointers
    // fBlock is true when this is called from AcceptBlock when a new best-block is added to the blockchain
    // fMiner is true when called from the internal bitcoin miner
    // ... both are false when called from CTransaction::AcceptToMemoryPool
    if (!IsCoinBase())
    {
        int64_t nValueIn = 0;
        int64_t nFees = 0;
        for (unsigned int i = 0; i < vin.size(); i++)
        {
            COutPoint prevout = vin[i].prevout;
            assert(inputs.count(prevout.hash) > 0);
            CTxIndex& txindex = inputs[prevout.hash].first;
            CTransaction& txPrev = inputs[prevout.hash].second;

            if (prevout.n >= txPrev.vout.size() || prevout.n >= txindex.vSpent.size())
                return DoS(100, error("ConnectInputs() : %s prevout.n out of range %d %" PRIszu " %" PRIszu " prev tx %s\n%s", GetHash().ToString().substr(0,10).c_str(), prevout.n, txPrev.vout.size(), txindex.vSpent.size(), prevout.hash.ToString().substr(0,10).c_str(), txPrev.ToString().c_str()));

            // If prev is coinbase or coinstake, check that it's matured
            if (txPrev.IsCoinBase() || txPrev.IsCoinStake())
                for (const CBlockIndex* pindex = pindexBlock; pindex && pindexBlock->nHeight - pindex->nHeight < nCoinbaseMaturity; pindex = pindex->pprev)
                    if (pindex->nBlockPos == txindex.pos.nBlockPos && pindex->nFile == txindex.pos.nFile)
                        return error("ConnectInputs() : tried to spend %s at depth %d", txPrev.IsCoinBase() ? "coinbase" : "coinstake", pindexBlock->nHeight - pindex->nHeight);

            // ppcoin: check transaction timestamp
            if (txPrev.nTime > nTime)
                return DoS(100, error("ConnectInputs() : transaction timestamp earlier than input transaction"));

            // Check for negative or overflow input values
            nValueIn += txPrev.vout[prevout.n].nValue;
            if (!MoneyRange(txPrev.vout[prevout.n].nValue) || !MoneyRange(nValueIn))
                return DoS(100, error("ConnectInputs() : txin values out of range"));

        }
        // The first loop above does all the inexpensive checks.
        // Only if ALL inputs pass do we perform expensive ECDSA signature checks.
        // Helps prevent CPU exhaustion attacks.
        for (unsigned int i = 0; i < vin.size(); i++)
        {
            COutPoint prevout = vin[i].prevout;
            assert(inputs.count(prevout.hash) > 0);
            CTxIndex& txindex = inputs[prevout.hash].first;
            CTransaction& txPrev = inputs[prevout.hash].second;

            // Check for conflicts (double-spend)
            // This doesn't trigger the DoS code on purpose; if it did, it would make it easier
            // for an attacker to attempt to split the network.
            if (!txindex.vSpent[prevout.n].IsNull())
			{
				if (fMiner)
				{
					msMiningErrorsExcluded += " ConnectInputs() : " + GetHash().GetHex() + " used at "
						+ txindex.vSpent[prevout.n].ToString() + ";   ";
					return false;
				}
				if (!txindex.vSpent[prevout.n].IsNull())
				{
					if (fTestNet && pindexBlock->nHeight < nGrandfather)
					{
						return fMiner ? false : true;
					}
					if (!fTestNet && pindexBlock->nHeight < nGrandfather)
					{
						return fMiner ? false : true;
					}
					if (TimerMain("ConnectInputs", 20))
					{
						CleanInboundConnections(false);
					}	
					
					if (fMiner) return false;
					return fDebug ? error("ConnectInputs() : %s prev tx already used at %s", GetHash().ToString().c_str(), txindex.vSpent[prevout.n].ToString().c_str()) : false;
				}

			}

            // Skip ECDSA signature verification when connecting blocks (fBlock=true)
            // before the last blockchain checkpoint. This is safe because block merkle hashes are
            // still computed and checked, and any change will be caught at the next checkpoint.

            if (!(fBlock && (nBestHeight < Checkpoints::GetTotalBlocksEstimate())))
            {
                // Verify signature
                if (!VerifySignature(txPrev, *this, i, 0))
                {
                    return DoS(100,error("ConnectInputs() : %s VerifySignature failed", GetHash().ToString().substr(0,10).c_str()));
                }
            }

            // Mark outpoints as spent
            txindex.vSpent[prevout.n] = posThisTx;

            // Write back
            if (fBlock || fMiner)
            {
                mapTestPool[prevout.hash] = txindex;
            }
        }

        if (!IsCoinStake())
        {
            if (nValueIn < GetValueOut())
			{
				printf("ConnectInputs(): VALUE IN < VALUEOUT \r\n");
                return DoS(100, error("ConnectInputs() : %s value in < value out", GetHash().ToString().substr(0,10).c_str()));
			}

            // Tally transaction fees
            int64_t nTxFee = nValueIn - GetValueOut();
            if (nTxFee < 0)
                return DoS(100, error("ConnectInputs() : %s nTxFee < 0", GetHash().ToString().substr(0,10).c_str()));

            // enforce transaction fees for every block
            if (nTxFee < GetMinFee())
                return fBlock? DoS(100, error("ConnectInputs() : %s not paying required fee=%s, paid=%s", GetHash().ToString().substr(0,10).c_str(), FormatMoney(GetMinFee()).c_str(), FormatMoney(nTxFee).c_str())) : false;

            nFees += nTxFee;
            if (!MoneyRange(nFees))
                return DoS(100, error("ConnectInputs() : nFees out of range"));
        }
    }

    return true;
}

bool CBlock::DisconnectBlock(CTxDB& txdb, CBlockIndex* pindex)
{

	// Disconnect in reverse order
	bool bDiscTxFailed = false;
    for (int i = vtx.size()-1; i >= 0; i--)
	{
        if (!vtx[i].DisconnectInputs(txdb))
		{
            bDiscTxFailed = true;
		}
	}

    // Update block index on disk without changing it in memory.
    // The memory index structure will be changed after the db commits.
    if (pindex->pprev)
    {
        CDiskBlockIndex blockindexPrev(pindex->pprev);
        blockindexPrev.hashNext = 0;
        if (!txdb.WriteBlockIndex(blockindexPrev))
            return error("DisconnectBlock() : WriteBlockIndex failed");
    }

    // ppcoin: clean up wallet after disconnecting coinstake
    BOOST_FOREACH(CTransaction& tx, vtx)
        SyncWithWallets(tx, this, false, false);

	StructCPID stCPID = GetLifetimeCPID(pindex->sCPID,"DisconnectBlock()");
	// We normally fail to disconnect a block if we can't find the previous input due to "DisconnectInputs() : ReadTxIndex failed".  Imo, I believe we should let this call succeed, otherwise a chain can never be re-organized in this circumstance.
	if (bDiscTxFailed && fDebug3) printf("!DisconnectBlock()::Failed, recovering. ");
    return true;
}



double BlockVersion(std::string v)
{
	if (v.length() < 10) return 0;
	std::string vIn = v.substr(1,7);
	boost::replace_all(vIn, ".", "");
	double ver1 = cdbl(vIn,0);
	return ver1;
}


std::string PubKeyToAddress(const CScript& scriptPubKey)
{
	//Converts a script Public Key to a Gridcoin wallet address
	txnouttype type;
    vector<CTxDestination> addresses;
    int nRequired;
    if (!ExtractDestinations(scriptPubKey, type, addresses, nRequired))
    {
        return "";
    }
	std::string address = "";
    BOOST_FOREACH(const CTxDestination& addr, addresses)
	{
		address = CBitcoinAddress(addr).ToString();
	}
	return address;
}

bool LoadSuperblock(std::string data, int64_t nTime, double height)
{
	 	WriteCache("superblock","magnitudes",ExtractXML(data,"<MAGNITUDES>","</MAGNITUDES>"),nTime);
		WriteCache("superblock","averages",ExtractXML(data,"<AVERAGES>","</AVERAGES>"),nTime);
		WriteCache("superblock","quotes",ExtractXML(data,"<QUOTES>","</QUOTES>"),nTime);
		WriteCache("superblock","all",data,nTime);
		WriteCache("superblock","block_number",RoundToString(height,0),nTime);
		return true;
}

std::string CharToString(char c)
{
	std::stringstream ss;
	std::string sOut = "";
	ss << c;
	ss >> sOut;
	return sOut;
}


template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << "0x" 
         << std::setfill ('0') << std::setw(sizeof(T)*2) 
         << std::hex << i;
  return stream.str();
}

std::string DoubleToHexStr(double d, int iPlaces)
{
	int nMagnitude = atoi(RoundToString(d,0).c_str()); 
	std::string hex_string = int_to_hex(nMagnitude);
	std::string sOut = "00000000" + hex_string;
	std::string sHex = sOut.substr(sOut.length()-iPlaces,iPlaces);
    return sHex;
}

int HexToInt(std::string sHex)
{
	int x;   
    std::stringstream ss;
    ss << std::hex << sHex;
    ss >> x;
	return x;
}
std::string ConvertHexToBin(std::string a)
{
	if (a.empty()) return "";
	std::string sOut = "";
	for (unsigned int x = 1; x <= a.length(); x += 2)
	{
	   std::string sChunk = a.substr(x-1,2);
	   int i = HexToInt(sChunk);
	   char c = (char)i;
	   sOut.push_back(c);
    }
	return sOut;
}


double ConvertHexToDouble(std::string hex)
{
	int d = HexToInt(hex);
	double dOut = (double)d;
	return dOut;
}


std::string ConvertBinToHex(std::string a) 
{
      if (a.empty()) return "0";
	  std::string sOut = "";
	  for (unsigned int x = 1; x <= a.length(); x++)
	  {
    	   char c = a[x-1];
		   int i = (int)c; 
		   std::string sHex = DoubleToHexStr((double)i,2);
		   sOut += sHex;
      }
      return sOut;
}

std::string UnpackBinarySuperblock(std::string sBlock)
{
	// 12-21-2015: R HALFORD: If the block is not binary, return the legacy format for backward compatibility
	std::string sBinary = ExtractXML(sBlock,"<BINARY>","</BINARY>");
	if (sBinary.empty()) return sBlock;
	std::string sZero = ExtractXML(sBlock,"<ZERO>","</ZERO>");
	double dZero = cdbl(sZero,0);
	// Binary data support structure:
	// Each CPID consumes 16 bytes and 2 bytes for magnitude: (Except CPIDs with zero magnitude - the count of those is stored in XML node <ZERO> to save space)
	// 1234567890123456MM
	// MM = Magnitude stored as 2 bytes
	// No delimiter between CPIDs, Step Rate = 18
	std::string sReconstructedMagnitudes = "";
	for (unsigned int x = 0; x < sBinary.length(); x += 18)
	{
		if (sBinary.length() >= x+18)
		{
			std::string bCPID = sBinary.substr(x,16);
			std::string bMagnitude = sBinary.substr(x+16,2);
			std::string sCPID = ConvertBinToHex(bCPID);
			std::string sHexMagnitude = ConvertBinToHex(bMagnitude);
			double dMagnitude = ConvertHexToDouble("0x" + sHexMagnitude);
			std::string sRow = sCPID + "," + RoundToString(dMagnitude,0) + ";";
			sReconstructedMagnitudes += sRow;
			// if (fDebug3) printf("\r\n HEX CPID %s, HEX MAG %s, dMag %f, Row %s   ",sCPID.c_str(),sHexMagnitude.c_str(),dMagnitude,sRow.c_str());
		}
	}
	// Append zero magnitude researchers so the beacon count matches
	for (double d0 = 1; d0 <= dZero; d0++)
	{
			std::string sZeroCPID = "0";
			std::string sRow1 = sZeroCPID + ",15;";
			sReconstructedMagnitudes += sRow1;
	}
	std::string sAverages   = ExtractXML(sBlock,"<AVERAGES>","</AVERAGES>");
	std::string sQuotes     = ExtractXML(sBlock,"<QUOTES>","</QUOTES>");
	std::string sReconstructedBlock = "<AVERAGES>" + sAverages + "</AVERAGES><QUOTES>" + sQuotes + "</QUOTES><MAGNITUDES>" + sReconstructedMagnitudes + "</MAGNITUDES>";
	return sReconstructedBlock;
}

std::string PackBinarySuperblock(std::string sBlock)
{

	std::string sMagnitudes = ExtractXML(sBlock,"<MAGNITUDES>","</MAGNITUDES>");
	std::string sAverages   = ExtractXML(sBlock,"<AVERAGES>","</AVERAGES>");
	std::string sQuotes     = ExtractXML(sBlock,"<QUOTES>","</QUOTES>");
	// For each CPID in the superblock, convert data to binary
	std::vector<std::string> vSuperblock = split(sMagnitudes.c_str(),";");
	std::string sBinary = "";
	double dZeroMagCPIDCount = 0;
	for (unsigned int i = 0; i < vSuperblock.size(); i++)
	{
			if (vSuperblock[i].length() > 1)
			{
				std::string sPrefix = "00000000000000000000000000000000000" + ExtractValue(vSuperblock[i],",",0);
				std::string sCPID = sPrefix.substr(sPrefix.length()-32,32);
				double magnitude = cdbl(ExtractValue("0"+vSuperblock[i],",",1),0);
				if (magnitude < 0)     magnitude=0;
				if (magnitude > 32767) magnitude = 32767;  // Ensure we do not blow out the binary space (technically we can handle 0-65535)
				std::string sBinaryCPID   = ConvertHexToBin(sCPID);
				std::string sHexMagnitude = DoubleToHexStr(magnitude,4);
				std::string sBinaryMagnitude = ConvertHexToBin(sHexMagnitude);
				std::string sBinaryEntry  = sBinaryCPID+sBinaryMagnitude;
				// if (fDebug3) printf("\r\n PackBinarySuperblock: DecMag %f HEX MAG %s bin_cpid_len %f bm_len %f be_len %f,",	magnitude,sHexMagnitude.c_str(),(double)sBinaryCPID.length(),(double)sBinaryMagnitude.length(),(double)sBinaryEntry.length());
				if (sCPID=="00000000000000000000000000000000")
				{
					dZeroMagCPIDCount += 1;
				}
				else
				{
					sBinary += sBinaryEntry;
				}

			}
	}
	std::string sReconstructedBinarySuperblock = "<ZERO>" + RoundToString(dZeroMagCPIDCount,0) + "</ZERO><BINARY>" + sBinary + "</BINARY><AVERAGES>" + sAverages + "</AVERAGES><QUOTES>" + sQuotes + "</QUOTES>";
	return sReconstructedBinarySuperblock;
}




double ClientVersionNew()
{
	double cv = BlockVersion(FormatFullVersion());
	return cv;
}


int64_t ReturnCurrentMoneySupply(CBlockIndex* pindexcurrent)
{
	if (pindexcurrent->pprev)
	{
		// If previous exists, and previous money supply > Genesis, OK to use it:
		if (pindexcurrent->pprev->nHeight > 11 && pindexcurrent->pprev->nMoneySupply > nGenesisSupply)
		{
			return pindexcurrent->pprev->nMoneySupply;
		}
	}
	// Special case where block height < 12, use standard old logic:
	if (pindexcurrent->nHeight < 12)
	{
		return (pindexcurrent->pprev? pindexcurrent->pprev->nMoneySupply : 0);
	}
	// At this point, either the last block pointer was NULL, or the client erased the money supply previously, fix it:
	CBlockIndex* pblockIndex = pindexcurrent;
	CBlockIndex* pblockMemory = pindexcurrent;
	int nMinDepth = (pindexcurrent->nHeight)-140000;
	if (nMinDepth < 12) nMinDepth=12;
	while (pblockIndex->nHeight > nMinDepth)
	{
			pblockIndex = pblockIndex->pprev;
			printf("Money Supply height %f",(double)pblockIndex->nHeight);

			if (pblockIndex == NULL || !pblockIndex->IsInMainChain()) continue;
			if (pblockIndex == pindexGenesisBlock)
			{
				return nGenesisSupply;
			}
	        if (pblockIndex->nMoneySupply > nGenesisSupply)
			{
				//Set index back to original pointer
				pindexcurrent = pblockMemory;
				//Return last valid money supply
				return pblockIndex->nMoneySupply;
			}
	}
	// At this point, we fall back to the old logic with a minimum of the genesis supply (should never happen - if it did, blockchain will need rebuilt anyway due to other fields being invalid):
	pindexcurrent = pblockMemory;
	return (pindexcurrent->pprev? pindexcurrent->pprev->nMoneySupply : nGenesisSupply);
}

bool CBlock::ConnectBlock(CTxDB& txdb, CBlockIndex* pindex, bool fJustCheck, bool fReorganizing)
{
    // Check it again in case a previous version let a bad block in, but skip BlockSig checking
    if (!CheckBlock("ConnectBlock",pindex->pprev->nHeight, 395*COIN, !fJustCheck, !fJustCheck, false,false))
	{
        printf("ConnectBlock::Failed - \r\n");
		return false;
	}
	//// issue here: it doesn't know the version
    unsigned int nTxPos;
    if (fJustCheck)
        // FetchInputs treats CDiskTxPos(1,1,1) as a special "refer to memorypool" indicator
        // Since we're just checking the block and not actually connecting it, it might not (and probably shouldn't) be on the disk to get the transaction from
        nTxPos = 1;
    else
        nTxPos = pindex->nBlockPos + ::GetSerializeSize(CBlock(), SER_DISK, CLIENT_VERSION) - (2 * GetSizeOfCompactSize(0)) + GetSizeOfCompactSize(vtx.size());

    map<uint256, CTxIndex> mapQueuedChanges;
    int64_t nFees = 0;
    int64_t nValueIn = 0;
    int64_t nValueOut = 0;
    int64_t nStakeReward = 0;
    unsigned int nSigOps = 0;
	double DPOR_Paid = 0;

	bool bIsDPOR = false;


    BOOST_FOREACH(CTransaction& tx, vtx)
    {
        uint256 hashTx = tx.GetHash();

        // Do not allow blocks that contain transactions which 'overwrite' older transactions,
        // unless those are already completely spent.
        // If such overwrites are allowed, coinbases and transactions depending upon those
        // can be duplicated to remove the ability to spend the first instance -- even after
        // being sent to another address.
        // See BIP30 and http://r6.ca/blog/20120206T005236Z.html for more information.
        // This logic is not necessary for memory pool transactions, as AcceptToMemoryPool
        // already refuses previously-known transaction ids entirely.
        // This rule was originally applied all blocks whose timestamp was after March 15, 2012, 0:00 UTC.
        // Now that the whole chain is irreversibly beyond that time it is applied to all blocks except the
        // two in the chain that violate it. This prevents exploiting the issue against nodes in their
        // initial block download.
        CTxIndex txindexOld;
        if (txdb.ReadTxIndex(hashTx, txindexOld)) {
            BOOST_FOREACH(CDiskTxPos &pos, txindexOld.vSpent)
                if (pos.IsNull())
                    return false;
        }

        nSigOps += tx.GetLegacySigOpCount();
        if (nSigOps > MAX_BLOCK_SIGOPS)
            return DoS(100, error("ConnectBlock[] : too many sigops"));

        CDiskTxPos posThisTx(pindex->nFile, pindex->nBlockPos, nTxPos);
        if (!fJustCheck)
            nTxPos += ::GetSerializeSize(tx, SER_DISK, CLIENT_VERSION);

        MapPrevTx mapInputs;
        if (tx.IsCoinBase())
		{
            nValueOut += tx.GetValueOut();
		}
        else
        {
            bool fInvalid;
            if (!tx.FetchInputs(txdb, mapQueuedChanges, true, false, mapInputs, fInvalid))
                return false;

            // Add in sigops done by pay-to-script-hash inputs;
            // this is to prevent a "rogue miner" from creating
            // an incredibly-expensive-to-validate block.
            nSigOps += tx.GetP2SHSigOpCount(mapInputs);
            if (nSigOps > MAX_BLOCK_SIGOPS)
                return DoS(100, error("ConnectBlock[] : too many sigops"));

            int64_t nTxValueIn = tx.GetValueIn(mapInputs);
            int64_t nTxValueOut = tx.GetValueOut();
            nValueIn += nTxValueIn;
            nValueOut += nTxValueOut;
            if (!tx.IsCoinStake())
                nFees += nTxValueIn - nTxValueOut;
            if (tx.IsCoinStake())
			{
                nStakeReward = nTxValueOut - nTxValueIn;
				if (tx.vout.size() > 3 && pindex->nHeight > nGrandfather) bIsDPOR = true;
				// ResearchAge: Verify vouts cannot contain any other payments except coinstake: PASS (GetValueOut returns the sum of all spent coins in the coinstake)
				if (IsResearchAgeEnabled(pindex->nHeight) && fDebug10)
				{
					int64_t nTotalCoinstake = 0;
					for (unsigned int i = 0; i < tx.vout.size(); i++)
					{
						nTotalCoinstake += tx.vout[i].nValue;
					}
					if (fDebug10) 	printf(" nHeight %f; nTCS %f; nTxValueOut %f     ",
						(double)pindex->nHeight,CoinToDouble(nTotalCoinstake),CoinToDouble(nTxValueOut));
				}

				//Options Support
				if (bOptionPaymentsEnabled)
				{
					// Disabled
				}
				else
				{
					// Verify no recipients exist after coinstake (Recipients start at output position 3 (0=Coinstake flag, 1=coinstake amount, 2=splitstake amount)
					if (bIsDPOR && pindex->nHeight > nGrandfather)
					{
						for (unsigned int i = 3; i < tx.vout.size(); i++)
						{
							std::string Recipient = PubKeyToAddress(tx.vout[i].scriptPubKey);
							double      Amount    = CoinToDouble(tx.vout[i].nValue);
							if (fDebug10) printf("Iterating Recipient #%f  %s with Amount %f \r\n,",(double)i,Recipient.c_str(),Amount);
  			  				if (Amount > 0)
							{
									if (fDebug3) printf("Iterating Recipient #%f  %s with Amount %f \r\n,",(double)i,Recipient.c_str(),Amount);
									printf("POR Payment results in an overpayment; Recipient %s, Amount %f \r\n",Recipient.c_str(), Amount);
		        					return DoS(50,error("POR Payment results in an overpayment; Recipient %s, Amount %f \r\n",
											Recipient.c_str(), Amount));
							}
						}
					}
				}
			}

            if (!tx.ConnectInputs(txdb, mapInputs, mapQueuedChanges, posThisTx, pindex, true, false))
                return false;
        }

        mapQueuedChanges[hashTx] = CTxIndex(posThisTx, tx.vout.size());
    }

    if (IsProofOfWork() && pindex->nHeight > nGrandfather)
    {
		int64_t nReward = GetProofOfWorkMaxReward(nFees,nTime,pindex->nHeight);
        // Check coinbase reward
        if (vtx[0].GetValueOut() > nReward)
            return DoS(50, error("ConnectBlock[] : coinbase reward exceeded (actual=%" PRId64 " vs calculated=%" PRId64 ")",
                   vtx[0].GetValueOut(),
                   nReward));
    }

	MiningCPID bb = DeserializeBoincBlock(vtx[0].hashBoinc);
	uint64_t nCoinAge = 0;

	double dStakeReward = CoinToDouble(nStakeReward+nFees) - DPOR_Paid; //DPOR Recipients checked above already
	double dStakeRewardWithoutFees = CoinToDouble(nStakeReward) - DPOR_Paid;

	if (fDebug) printf("Stake Reward of %f , DPOR PAID %f    ",dStakeReward,DPOR_Paid);

    if (IsProofOfStake() && pindex->nHeight > nGrandfather)
    {
	    // ppcoin: coin stake tx earns reward instead of paying fee
        if (!vtx[1].GetCoinAge(txdb, nCoinAge))
            return error("ConnectBlock[] : %s unable to get coin age for coinstake", vtx[1].GetHash().ToString().substr(0,10).c_str());

		double dCalcStakeReward = CoinToDouble(GetProofOfStakeMaxReward(nCoinAge, nFees, nTime));

		if (dStakeReward > dCalcStakeReward+1 && !IsResearchAgeEnabled(pindex->nHeight))
            return DoS(1, error("ConnectBlock[] : coinstake pays above maximum (actual= %f, vs calculated=%f )", dStakeReward, dCalcStakeReward));

		//9-3-2015
		double dMaxResearchAgeReward = CoinToDouble(GetMaximumBoincSubsidy(nTime) * COIN * 255);

		if (bb.ResearchSubsidy > dMaxResearchAgeReward && IsResearchAgeEnabled(pindex->nHeight))
            return DoS(1, error("ConnectBlock[ResearchAge] : Coinstake pays above maximum (actual= %f, vs calculated=%f )", dStakeRewardWithoutFees, dMaxResearchAgeReward));

		if (bb.cpid=="INVESTOR" && dStakeReward > 1)
		{
			double OUT_POR = 0;
			double OUT_INTEREST_OWED = 0;

			double dAccrualAge = 0;
    		double dAccrualMagnitudeUnit = 0;
	    	double dAccrualMagnitude = 0;

			double dCalculatedResearchReward = CoinToDouble(GetProofOfStakeReward(nCoinAge, nFees, bb.cpid, true, 1, nTime,
				    pindex, "connectblock_investor",
					OUT_POR, OUT_INTEREST_OWED, dAccrualAge, dAccrualMagnitudeUnit, dAccrualMagnitude));
			if (dStakeReward > (OUT_INTEREST_OWED+1+nFees) )
			{
					return DoS(10, error("ConnectBlock[] : Investor Reward pays too much : cpid %s (actual %f vs calculated %f), dCalcResearchReward %f, Fees %f",
					bb.cpid.c_str(), dStakeReward, OUT_INTEREST_OWED, dCalculatedResearchReward, (double)nFees));
			}
		}

 	}


	AddCPIDBlockHash(bb.cpid, pindex->GetBlockHash());

    // Track money supply and mint amount info
    pindex->nMint = nValueOut - nValueIn + nFees;
	if (fDebug10) printf (".TMS.");

    pindex->nMoneySupply = ReturnCurrentMoneySupply(pindex) + nValueOut - nValueIn;

	// Gridcoin: Store verified magnitude and CPID in block index (7-11-2015)
	if (pindex->nHeight > nNewIndex2)
	{
		pindex->sCPID  = bb.cpid;
		pindex->nMagnitude = bb.Magnitude;
		pindex->nResearchSubsidy = bb.ResearchSubsidy;
		pindex->nInterestSubsidy = bb.InterestSubsidy;
		pindex->nIsSuperBlock =  (bb.superblock.length() > 20) ? 1 : 0;
		// Must scan transactions after CoinStake to know if this is a contract.
		int iPos = 0;
		pindex->nIsContract = 0;
		BOOST_FOREACH(const CTransaction &tx, vtx)
		{
			if (tx.hashBoinc.length() > 3 && iPos > 0)
			{
				pindex->nIsContract = 1;
				break;
			}
			iPos++;
		}
		pindex->sGRCAddress = bb.GRCAddress;
	}

	double mint = CoinToDouble(pindex->nMint);
    double PORDiff = GetBlockDifficulty(nBits);

	if (pindex->nHeight > nGrandfather && !fReorganizing)
	{
		// Block Spamming
		if (mint < MintLimiter(PORDiff,bb.RSAWeight,bb.cpid,GetBlockTime()))
		{
			return error("CheckProofOfStake[] : Mint too Small, %f",(double)mint);
		}

		if (mint == 0) return error("CheckProofOfStake[] : Mint is ZERO! %f",(double)mint);

		double OUT_POR = 0;
		double OUT_INTEREST = 0;
		double dAccrualAge = 0;
		double dMagnitudeUnit = 0;
		double dAvgMagnitude = 0;

	    // ResearchAge 1: 
		GetProofOfStakeReward(nCoinAge, nFees, bb.cpid, true, 1, nTime,
			pindex, "connectblock_researcher", OUT_POR, OUT_INTEREST, dAccrualAge, dMagnitudeUnit, dAvgMagnitude);
		if (bb.cpid != "INVESTOR" && dStakeReward > 1)
		{
			
			    //ResearchAge: Since the best block may increment before the RA is connected but After the RA is computed, the ResearchSubsidy can sometimes be slightly smaller than we calculate here due to the RA timespan increasing.  So we will allow for time shift before rejecting the block.
			    double dDrift = IsResearchAgeEnabled(pindex->nHeight) ? bb.ResearchSubsidy*.15 : 1;
				if (IsResearchAgeEnabled(pindex->nHeight) && dDrift < 10) dDrift = 10;

				if ((bb.ResearchSubsidy + bb.InterestSubsidy + dDrift) < dStakeRewardWithoutFees)
				{
						return error("ConnectBlock[] : Researchers Interest %f + Research %f + TimeDrift %f and total Mint %f, [StakeReward] <> %f, with Out_Interest %f, OUT_POR %f, Fees %f, DPOR %f  for CPID %s does not match calculated research subsidy",
							(double)bb.InterestSubsidy,(double)bb.ResearchSubsidy,dDrift,CoinToDouble(mint),dStakeRewardWithoutFees,
							(double)OUT_INTEREST,(double)OUT_POR,CoinToDouble(nFees),(double)DPOR_Paid,bb.cpid.c_str());

				}
				if (IsResearchAgeEnabled(pindex->nHeight) && BlockNeedsChecked(nTime))
				{
						if (dStakeReward > ((OUT_POR*1.25)+OUT_INTEREST+1+CoinToDouble(nFees)))
						{
							StructCPID st1 = GetLifetimeCPID(pindex->sCPID,"ConnectBlock()");
							GetProofOfStakeReward(nCoinAge, nFees, bb.cpid, true, 2, nTime,
										pindex, "connectblock_researcher_doublecheck", OUT_POR, OUT_INTEREST, dAccrualAge, dMagnitudeUnit, dAvgMagnitude);
							if (dStakeReward > ((OUT_POR*1.25)+OUT_INTEREST+1+CoinToDouble(nFees)))
							{

								if (fDebug3) printf("ConnectBlockError[ResearchAge] : Researchers Reward Pays too much : Interest %f and Research %f and StakeReward %f, OUT_POR %f, with Out_Interest %f for CPID %s ",
									(double)bb.InterestSubsidy,(double)bb.ResearchSubsidy,dStakeReward,(double)OUT_POR,(double)OUT_INTEREST,bb.cpid.c_str());

								return DoS(10,error("ConnectBlock[ResearchAge] : Researchers Reward Pays too much : Interest %f and Research %f and StakeReward %f, OUT_POR %f, with Out_Interest %f for CPID %s ",
									(double)bb.InterestSubsidy,(double)bb.ResearchSubsidy,dStakeReward,(double)OUT_POR,(double)OUT_INTEREST,bb.cpid.c_str()));
							}
						}
				}
		}

		//Approve first coinstake in DPOR block
		if (bb.cpid != "INVESTOR" && IsLockTimeWithinMinutes(GetBlockTime(),15) && !IsResearchAgeEnabled(pindex->nHeight))
		{
			    if (bb.ResearchSubsidy > (GetOwedAmount(bb.cpid)+1))
				{
						bDoTally=true;
					    if (bb.ResearchSubsidy > (GetOwedAmount(bb.cpid)+1))
						{
							StructCPID strUntrustedHost = GetInitializedStructCPID2(bb.cpid,mvMagnitudes);
							if (bb.ResearchSubsidy > strUntrustedHost.totalowed)
							{
								double deficit = strUntrustedHost.totalowed - bb.ResearchSubsidy;
								if ( (deficit < -500 && strUntrustedHost.Accuracy > 10) || (deficit < -150 && strUntrustedHost.Accuracy > 5) || deficit < -50)
								{
										printf("ConnectBlock[] : Researchers Reward results in deficit of %f for CPID %s with trust level of %f - (Submitted Research Subsidy %f vs calculated=%f) Hash: %s",
										 deficit, bb.cpid.c_str(), (double)strUntrustedHost.Accuracy, bb.ResearchSubsidy,
										 OUT_POR, vtx[0].hashBoinc.c_str());
								}
								else
								{
									return error("ConnectBlock[] : Researchers Reward for CPID %s pays too much - (Submitted Research Subsidy %f vs calculated=%f) Hash: %s",
										bb.cpid.c_str(), bb.ResearchSubsidy,
										OUT_POR, vtx[0].hashBoinc.c_str());
								}
							}
					}
				}
		}

	}

	//Gridcoin: Maintain network consensus for Payments and Neural popularity:  (As of 7-5-2015 this is now done exactly every 30 blocks)

	//DPOR - 6/12/2015 - Reject superblocks not hashing to the supermajority:

	if (bb.superblock.length() > 20)
	{
		if (pindex->nHeight > nGrandfather && !fReorganizing)
		{
			// 12-20-2015 : Add support for Binary Superblocks
			std::string superblock = UnpackBinarySuperblock(bb.superblock);
			std::string neural_hash = GetQuorumHash(superblock);
			std::string legacy_neural_hash = RetrieveMd5(superblock);
			double popularity = 0;
			std::string consensus_hash = GetNeuralNetworkSupermajorityHash(popularity);
			// Only reject superblock when it is new And when QuorumHash of Block != the Popular Quorum Hash:
			if (IsLockTimeWithinMinutes(GetBlockTime(),15)  && !fColdBoot)
			{
				if (!VerifySuperblock(superblock,pindex->nHeight))
				{
					return error("ConnectBlock[] : Superblock avg mag below 10; SuperblockHash: %s, Consensus Hash: %s",
										neural_hash.c_str(), consensus_hash.c_str());
				}
				if (!IsResearchAgeEnabled(pindex->nHeight))
				{
					if (consensus_hash != neural_hash && consensus_hash != legacy_neural_hash)
					{
						return error("ConnectBlock[] : Superblock hash does not match consensus hash; SuperblockHash: %s, Consensus Hash: %s",
										neural_hash.c_str(), consensus_hash.c_str());
					}
				}
				else
				{
					if (consensus_hash != neural_hash)
					{
						return error("ConnectBlock[] : Superblock hash does not match consensus hash; SuperblockHash: %s, Consensus Hash: %s",
										neural_hash.c_str(), consensus_hash.c_str());
					}
				}

			}
		}


			//If we are out of sync, and research age is enabled, and the superblock is valid, load it now, so we can continue checking blocks accurately
			if ((OutOfSyncByAge() || fColdBoot || fReorganizing) && IsResearchAgeEnabled(pindex->nHeight) && pindex->nHeight > nGrandfather)
			{
				    if (bb.superblock.length() > 20)
					{
						    std::string superblock = UnpackBinarySuperblock(bb.superblock);
							if (VerifySuperblock(superblock,pindex->nHeight))
							{
										LoadSuperblock(superblock,pindex->nTime,pindex->nHeight);
										if (fDebug3) printf("ConnectBlock(): Superblock Loaded %f \r\n",(double)pindex->nHeight);
										/*  Reserved for future use:
											bNetAveragesLoaded=false;
											nLastTallied = 0;
											BsyWaitForTally();
										*/
										if (!fColdBoot)
										{
											bDoTally = true;
										}
							}
							else
							{
								if (fDebug3) printf("ConnectBlock(): Superblock Not Loaded %f\r\n",(double)pindex->nHeight);
							}
					}
			}



		/*
			-- Normal Superblocks are loaded 15 blocks later
		*/
	}

	//  End of Network Consensus

	// Gridcoin: Track payments to CPID, and last block paid
	if (!bb.cpid.empty() && bb.cpid != "INVESTOR" && pindex->nHeight > nNewIndex2)
	{
		StructCPID stCPID = GetInitializedStructCPID2(bb.cpid,mvResearchAge);
		stCPID.InterestSubsidy += bb.InterestSubsidy;
		stCPID.ResearchSubsidy += bb.ResearchSubsidy;

		if (((double)pindex->nHeight) > stCPID.LastBlock && pindex->nResearchSubsidy > 0)
		{
				stCPID.LastBlock = (double)pindex->nHeight;
				stCPID.BlockHash = pindex->GetBlockHash().GetHex();
		}

		if (pindex->nMagnitude > 0)
		{
				stCPID.Accuracy++;
				stCPID.TotalMagnitude += pindex->nMagnitude;
				stCPID.ResearchAverageMagnitude = stCPID.TotalMagnitude/(stCPID.Accuracy+.01);
		}

		if (((double)pindex->nTime) < stCPID.LowLockTime)  stCPID.LowLockTime = (double)pindex->nTime;
		if (((double)pindex->nTime) > stCPID.HighLockTime) stCPID.HighLockTime = (double)pindex->nTime;

		mvResearchAge[bb.cpid]=stCPID;
	}

	if (!txdb.WriteBlockIndex(CDiskBlockIndex(pindex)))
        return error("Connect() : WriteBlockIndex for pindex failed");

	if (pindex->nHeight % 5 == 0 && pindex->nHeight > 100)
	{
		std::string errors1 = "";
		LoadAdminMessages(false,errors1);
	}

	// Slow down Retallying when in RA mode so we minimize disruption of the network
	if ( (pindex->nHeight % 60 == 0) && IsResearchAgeEnabled(pindex->nHeight) && BlockNeedsChecked(pindex->nTime))
	{
		if (fDebug3) printf("\r\n*BusyWaitForTally*\r\n");
		BusyWaitForTally();
	}


	if (IsResearchAgeEnabled(pindex->nHeight) && !OutOfSyncByAge()) 
	{
			fColdBoot = false;
			bDoTally=true;
	}

    if (fJustCheck)
        return true;

    // Write queued txindex changes
    for (map<uint256, CTxIndex>::iterator mi = mapQueuedChanges.begin(); mi != mapQueuedChanges.end(); ++mi)
    {
        if (!txdb.UpdateTxIndex((*mi).first, (*mi).second))
            return error("ConnectBlock[] : UpdateTxIndex failed");
    }

    // Update block index on disk without changing it in memory.
    // The memory index structure will be changed after the db commits.
    if (pindex->pprev)
    {
        CDiskBlockIndex blockindexPrev(pindex->pprev);
        blockindexPrev.hashNext = pindex->GetBlockHash();
        if (!txdb.WriteBlockIndex(blockindexPrev))
            return error("ConnectBlock[] : WriteBlockIndex failed");
    }

    // Watch for transactions paying to me
    BOOST_FOREACH(CTransaction& tx, vtx)
        SyncWithWallets(tx, this, true);

    return true;
}




bool static Reorganize(CTxDB& txdb, CBlockIndex* pindexNew)
{
    printf("REORGANIZE\n");
	// Find the fork
    CBlockIndex* pfork = pindexBest;
    CBlockIndex* plonger = pindexNew;
    while (pfork != plonger)
    {
        while (plonger->nHeight > pfork->nHeight)
            if (!(plonger = plonger->pprev))
                return error("Reorganize() : plonger->pprev is null");
        if (pfork == plonger)
            break;
        if (!(pfork = pfork->pprev))
            return error("Reorganize() : pfork->pprev is null");
    }

    // List of what to disconnect
	vector<CBlockIndex*> vDisconnect;
    for (CBlockIndex* pindex = pindexBest; pindex != pfork; pindex = pindex->pprev)
        vDisconnect.push_back(pindex);

    // List of what to connect
    vector<CBlockIndex*> vConnect;
    for (CBlockIndex* pindex = pindexNew; pindex != pfork; pindex = pindex->pprev)
        vConnect.push_back(pindex);
    reverse(vConnect.begin(), vConnect.end());

    printf("REORGANIZE: Disconnect %" PRIszu " blocks; %s..%s\n", vDisconnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexBest->GetBlockHash().ToString().substr(0,20).c_str());
    printf("REORGANIZE: Connect %" PRIszu " blocks; %s..%s\n", vConnect.size(), pfork->GetBlockHash().ToString().substr(0,20).c_str(), pindexNew->GetBlockHash().ToString().substr(0,20).c_str());

	if (vDisconnect.size() > 0)
	{
		//Block was disconnected - User is Re-eligibile for staking

		StructCPID sMag = GetInitializedStructCPID2(GlobalCPUMiningCPID.cpid,mvMagnitudes);

		if (sMag.initialized)
		{
			sMag.LastPaymentTime = 0;
			mvMagnitudes[GlobalCPUMiningCPID.cpid]=sMag;
		}
		nLastBlockSolved = 0;
	}
	printf("REORGANIZE Disc Size %f",(double)vDisconnect.size());

    // Disconnect shorter branch
    list<CTransaction> vResurrect;
    BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
    {
        CBlock block;
        if (!block.ReadFromDisk(pindex))
            return error("Reorganize() : ReadFromDisk for disconnect failed");
        if (!block.DisconnectBlock(txdb, pindex))
            return error("Reorganize() : DisconnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());

        // Queue memory transactions to resurrect.
        // We only do this for blocks after the last checkpoint (reorganisation before that
        // point should only happen with -reindex/-loadblock, or a misbehaving peer.
        BOOST_REVERSE_FOREACH(const CTransaction& tx, block.vtx)
            if (!(tx.IsCoinBase() || tx.IsCoinStake()) && pindex->nHeight > Checkpoints::GetTotalBlocksEstimate())
                vResurrect.push_front(tx);
    }

    // Connect longer branch
    vector<CTransaction> vDelete;
    for (unsigned int i = 0; i < vConnect.size(); i++)
    {
        CBlockIndex* pindex = vConnect[i];
        CBlock block;
        if (!block.ReadFromDisk(pindex))
            return error("Reorganize() : ReadFromDisk for connect failed");
        if (!block.ConnectBlock(txdb, pindex, false, true))
        {
            // Invalid block
            return error("Reorganize() : ConnectBlock %s failed", pindex->GetBlockHash().ToString().substr(0,20).c_str());
        }

        // Queue memory transactions to delete
        BOOST_FOREACH(const CTransaction& tx, block.vtx)
            vDelete.push_back(tx);

		if (!IsResearchAgeEnabled(pindex->nHeight))
		{
			//MiningCPID bb = GetInitializedMiningCPID(pindex->GetBlockHash().GetHex(), mvBlockIndex);
	   		//bb = DeserializeBoincBlock(block.vtx[0].hashBoinc);
			//mvBlockIndex[pindex->GetBlockHash().GetHex()] = bb;
		}
    }

    if (!txdb.WriteHashBestChain(pindexNew->GetBlockHash()))
        return error("Reorganize() : WriteHashBestChain failed");

    // Make sure it's successfully written to disk before changing memory structure
    if (!txdb.TxnCommit())
        return error("Reorganize() : TxnCommit failed");

    // Disconnect shorter branch
    BOOST_FOREACH(CBlockIndex* pindex, vDisconnect)
	{
        if (pindex->pprev)
		{
            pindex->pprev->pnext = NULL;
			//if (IsResearchAgeEnabled(pindex->nHeight)) 	StructCPID st1 = GetLifetimeCPID(pindex->pprev->sCPID,"Reorganize()");
		}
	}

    // Connect longer branch
    BOOST_FOREACH(CBlockIndex* pindex, vConnect)
	{
        if (pindex->pprev)
		{
            pindex->pprev->pnext = pindex;
			//if (IsResearchAgeEnabled(pindex->nHeight)) 	StructCPID st2 = GetLifetimeCPID(pindex->pprev->sCPID,"Reorganize()::ConnectLongerBranch()");
		}
	}

    // Resurrect memory transactions that were in the disconnected branch
    BOOST_FOREACH(CTransaction& tx, vResurrect)
        AcceptToMemoryPool(mempool, tx, NULL);

    // Delete redundant memory transactions that are in the connected branch
    BOOST_FOREACH(CTransaction& tx, vDelete)
	{
        mempool.remove(tx);
        mempool.removeConflicts(tx);
    }

	// Gridcoin: Now that the chain is back in order, Fix the researchers who were disrupted:
	
    printf("REORGANIZE: done\n");
    return true;
}


bool CleanChain()
{
	CTxDB txdb;
   if (!txdb.TxnBegin())
        return error("CleanChain() : TxnBegin failed");

	if (nBestHeight < 1000) return true;

    printf("\r\n** CLEAN CHAIN **\r\n");
	// Roll back a few blocks from best height
	printf(" Current best height %f ",(double)pindexBest->nHeight);
	CBlockIndex* pfork = pindexBest->pprev;
	CBlockIndex* pindexNew = pfork->pprev;
	printf(" Target height %f ",(double)pfork->nHeight);

	if (!Reorganize(txdb, pfork))
	{
					printf("Failed to Reorganize during Attempt #%f \r\n",(double)1);
					txdb.TxnAbort();
					//InvalidChainFound(pindexNew);
					return false;
	}
	else
	{
			CBlock blockNew;
			if (!blockNew.ReadFromDisk(pindexNew))
			{
				printf("CleanChain(): Fatal Error while reading new best block.\r\n");
				return false;
			}

			if (!blockNew.SetBestChain(txdb, pindexNew))
			{
				return error("CleanChain(): Fatal Error while setting best chain.\r\n");
			}

			printf(" Clean Chain succeeded. ");
	}
	bool fResult = AskForOutstandingBlocks(uint256(0));
	return true;

}



void SetAdvisory()
{
	CheckpointsMode = Checkpoints::ADVISORY;

}

bool InAdvisory()
{
	return (CheckpointsMode == Checkpoints::ADVISORY);
}

// Called from inside SetBestChain: attaches a block to the new best chain being built
bool CBlock::SetBestChainInner(CTxDB& txdb, CBlockIndex *pindexNew, bool fReorganizing)
{
    uint256 hash = GetHash();

    // Adding to current best branch
    if (!ConnectBlock(txdb, pindexNew, false, fReorganizing) || !txdb.WriteHashBestChain(hash))
    {
        txdb.TxnAbort();
		if (fDebug3) printf("Invalid Chain Found.  Invalid block %s\r\n",hash.GetHex().c_str());
        InvalidChainFound(pindexNew);
        return false;
    }
    if (!txdb.TxnCommit())
        return error("SetBestChain() : TxnCommit failed");

    // Add to current best branch
    pindexNew->pprev->pnext = pindexNew;

    // Delete redundant memory transactions
    BOOST_FOREACH(CTransaction& tx, vtx)
        mempool.remove(tx);

    return true;
}

bool CBlock::SetBestChain(CTxDB& txdb, CBlockIndex* pindexNew)
{
    uint256 hash = GetHash();

    if (!txdb.TxnBegin())
        return error("SetBestChain() : TxnBegin failed");

    if (pindexGenesisBlock == NULL && hash == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet))
    {
        txdb.WriteHashBestChain(hash);
        if (!txdb.TxnCommit())
            return error("SetBestChain() : TxnCommit failed");
        pindexGenesisBlock = pindexNew;
    }
    else if (hashPrevBlock == hashBestChain)
    {
        if (!SetBestChainInner(txdb, pindexNew, false))
		{
			//int nResult = 0;
            return error("SetBestChain() : SetBestChainInner failed");
		}
    }
    else
    {
        // the first block in the new chain that will cause it to become the new best chain
        CBlockIndex *pindexIntermediate = pindexNew;
		// list of blocks that need to be connected afterwards
        std::vector<CBlockIndex*> vpindexSecondary;
		printf("\r\n**Reorganize**");

		//10-6-2015 Make Reorganize work more gracefully - try up to 5 times to reorganize, each with an intermediate further back
        for (int iRegression = 0; iRegression < 5; iRegression++)
		{
			int rollback = iRegression * 100;

		    // Reorganize is costly in terms of db load, as it works in a single db transaction.
			// Try to limit how much needs to be done inside
			int rolled_back = 1;
			while (pindexIntermediate->pprev && pindexIntermediate->pprev->nChainTrust > pindexBest->nChainTrust && rolled_back < rollback)
			{
				vpindexSecondary.push_back(pindexIntermediate);
				pindexIntermediate = pindexIntermediate->pprev;
				if (pindexIntermediate==pindexGenesisBlock) break;
				rolled_back++;
			}

			if (!vpindexSecondary.empty())
			printf("\r\nReorganizing Attempt #%f, regression to block #%f \r\n",(double)iRegression+1,(double)pindexIntermediate->nHeight);

            printf("Postponing %" PRIszu " reconnects\n", vpindexSecondary.size());
			if (iRegression==4 && !Reorganize(txdb, pindexIntermediate))
			{
					printf("Failed to Reorganize during Attempt #%f \r\n",(double)iRegression+1);
					txdb.TxnAbort();
					InvalidChainFound(pindexNew);
					printf("\r\nReorg BusyWait\r\n");
					BusyWaitForTally();
				 	REORGANIZE_FAILED++;
					return error("SetBestChain() : Reorganize failed");
			}
		}
		// Switch to new best branch
		REORGANIZE_FAILED=0;

        // Connect further blocks
        BOOST_REVERSE_FOREACH(CBlockIndex *pindex, vpindexSecondary)
        {
            CBlock block;
            if (!block.ReadFromDisk(pindex))
            {
                printf("SetBestChain() : ReadFromDisk failed\n");
                break;
            }
            if (!txdb.TxnBegin()) {
                printf("SetBestChain() : TxnBegin 2 failed\n");
                break;
            }
            // errors now are not fatal, we still did a reorganisation to a new chain in a valid way
            if (!block.SetBestChainInner(txdb, pindex, true))
                break;
        }
    }

    // Update best block in wallet (so we can detect restored wallets)
    bool fIsInitialDownload = IsInitialBlockDownload();
    if (!fIsInitialDownload)
    {
        const CBlockLocator locator(pindexNew);
        ::SetBestChain(locator);
    }

    // New best block
    hashBestChain = hash;
    pindexBest = pindexNew;
    pblockindexFBBHLast = NULL;
    nBestHeight = pindexBest->nHeight;
    nBestChainTrust = pindexNew->nChainTrust;
    nTimeBestReceived =  GetAdjustedTime();
    nTransactionsUpdated++;

    uint256 nBestBlockTrust = pindexBest->nHeight != 0 ? (pindexBest->nChainTrust - pindexBest->pprev->nChainTrust) : pindexBest->nChainTrust;

	if (fDebug)
	{
		printf("{SBC} SetBestChain: new best=%s  height=%d  trust=%s  blocktrust=%" PRId64 "  date=%s\n",
		  hashBestChain.ToString().substr(0,20).c_str(), nBestHeight,
		  CBigNum(nBestChainTrust).ToString().c_str(),
          nBestBlockTrust.Get64(),
          DateTimeStrFormat("%x %H:%M:%S", pindexBest->GetBlockTime()).c_str());
	}
	else
	{
		printf("{SBC} new best=%s  height=%d ; ",hashBestChain.ToString().c_str(), nBestHeight);
		nLastBestHeight = nBestHeight;
	}

    // Check the version of the last 100 blocks to see if we need to upgrade:
    if (!fIsInitialDownload)
    {
        int nUpgraded = 0;
        const CBlockIndex* pindex = pindexBest;
        for (int i = 0; i < 100 && pindex != NULL; i++)
        {
            if (pindex->nVersion > CBlock::CURRENT_VERSION)
                ++nUpgraded;
            pindex = pindex->pprev;
        }
        if (nUpgraded > 0)
            printf("SetBestChain: %d of last 100 blocks above version %d\n", nUpgraded, CBlock::CURRENT_VERSION);
        if (nUpgraded > 100/2)
            // strMiscWarning is read by GetWarnings(), called by Qt and the JSON-RPC code to warn the user:
            strMiscWarning = _("Warning: This version is obsolete, upgrade required!");
    }

    std::string strCmd = GetArg("-blocknotify", "");

    if (!fIsInitialDownload && !strCmd.empty())
    {
        boost::replace_all(strCmd, "%s", hashBestChain.GetHex());
        boost::thread t(runCommand, strCmd); // thread runs free
    }
	REORGANIZE_FAILED=0;

    return true;
}

// ppcoin: total coin age spent in transaction, in the unit of coin-days.
// Only those coins meeting minimum age requirement counts. As those
// transactions not in main chain are not currently indexed so we
// might not find out about their coin age. Older transactions are
// guaranteed to be in main chain by sync-checkpoint. This rule is
// introduced to help nodes establish a consistent view of the coin
// age (trust score) of competing branches.
bool CTransaction::GetCoinAge(CTxDB& txdb, uint64_t& nCoinAge) const
{
    CBigNum bnCentSecond = 0;  // coin age in the unit of cent-seconds
    nCoinAge = 0;

    if (IsCoinBase())
        return true;

    BOOST_FOREACH(const CTxIn& txin, vin)
    {
        // First try finding the previous transaction in database
        CTransaction txPrev;
        CTxIndex txindex;
        if (!txPrev.ReadFromDisk(txdb, txin.prevout, txindex))
            continue;  // previous transaction not in main chain
        if (nTime < txPrev.nTime)
            return false;  // Transaction timestamp violation

        // Read block header
        CBlock block;
        if (!block.ReadFromDisk(txindex.pos.nFile, txindex.pos.nBlockPos, false))
            return false; // unable to read block of previous transaction
        if (block.GetBlockTime() + nStakeMinAge > nTime)
            continue; // only count coins meeting min age requirement

        int64_t nValueIn = txPrev.vout[txin.prevout.n].nValue;
        bnCentSecond += CBigNum(nValueIn) * (nTime-txPrev.nTime) / CENT;

        if (fDebug && GetBoolArg("-printcoinage"))
            printf("coin age nValueIn=%" PRId64 " nTimeDiff=%d bnCentSecond=%s\n", nValueIn, nTime - txPrev.nTime, bnCentSecond.ToString().c_str());
    }

    CBigNum bnCoinDay = bnCentSecond * CENT / COIN / (24 * 60 * 60);
    if (fDebug && GetBoolArg("-printcoinage"))
        printf("coin age bnCoinDay=%s\n", bnCoinDay.ToString().c_str());
    nCoinAge = bnCoinDay.getuint64();
    return true;
}

// ppcoin: total coin age spent in block, in the unit of coin-days.
bool CBlock::GetCoinAge(uint64_t& nCoinAge) const
{
    nCoinAge = 0;

    CTxDB txdb("r");
    BOOST_FOREACH(const CTransaction& tx, vtx)
    {
        uint64_t nTxCoinAge;
        if (tx.GetCoinAge(txdb, nTxCoinAge))
            nCoinAge += nTxCoinAge;
        else
            return false;
    }

    if (nCoinAge == 0) // block coin age minimum 1 coin-day
        nCoinAge = 1;
    if (fDebug && GetBoolArg("-printcoinage"))
        printf("block coin age total nCoinDays=%" PRId64 "\n", nCoinAge);
    return true;
}

bool CBlock::AddToBlockIndex(unsigned int nFile, unsigned int nBlockPos, const uint256& hashProof)
{
    // Check for duplicate
    uint256 hash = GetHash();
    if (mapBlockIndex.count(hash))
        return error("AddToBlockIndex() : %s already exists", hash.ToString().substr(0,20).c_str());

    // Construct new block index object
    CBlockIndex* pindexNew = new CBlockIndex(nFile, nBlockPos, *this);
    if (!pindexNew)
        return error("AddToBlockIndex() : new CBlockIndex failed");
    pindexNew->phashBlock = &hash;
    map<uint256, CBlockIndex*>::iterator miPrev = mapBlockIndex.find(hashPrevBlock);
    if (miPrev != mapBlockIndex.end())
    {
        pindexNew->pprev = (*miPrev).second;
        pindexNew->nHeight = pindexNew->pprev->nHeight + 1;
    }

    // ppcoin: compute chain trust score
    pindexNew->nChainTrust = (pindexNew->pprev ? pindexNew->pprev->nChainTrust : 0) + pindexNew->GetBlockTrust();

    // ppcoin: compute stake entropy bit for stake modifier
    if (!pindexNew->SetStakeEntropyBit(GetStakeEntropyBit()))
        return error("AddToBlockIndex() : SetStakeEntropyBit() failed");

    // Record proof hash value
    pindexNew->hashProof = hashProof;

    // ppcoin: compute stake modifier
    uint64_t nStakeModifier = 0;
    bool fGeneratedStakeModifier = false;
    if (!ComputeNextStakeModifier(pindexNew->pprev, nStakeModifier, fGeneratedStakeModifier))
	{
        printf("AddToBlockIndex() : ComputeNextStakeModifier() failed");
	}
    pindexNew->SetStakeModifier(nStakeModifier, fGeneratedStakeModifier);
    pindexNew->nStakeModifierChecksum = GetStakeModifierChecksum(pindexNew);

    // Add to mapBlockIndex
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.insert(make_pair(hash, pindexNew)).first;
    if (pindexNew->IsProofOfStake())
        setStakeSeen.insert(make_pair(pindexNew->prevoutStake, pindexNew->nStakeTime));
    pindexNew->phashBlock = &((*mi).first);

    // Write to disk block index
    CTxDB txdb;
    if (!txdb.TxnBegin())
        return false;
    txdb.WriteBlockIndex(CDiskBlockIndex(pindexNew));
    if (!txdb.TxnCommit())
        return false;

    LOCK(cs_main);

    // New best
    if (pindexNew->nChainTrust > nBestChainTrust)
        if (!SetBestChain(txdb, pindexNew))
            return false;

    if (pindexNew == pindexBest)
    {
        // Notify UI to display prev block's coinbase if it was ours
        static uint256 hashPrevBestCoinBase;
        UpdatedTransaction(hashPrevBestCoinBase);
        hashPrevBestCoinBase = vtx[0].GetHash();
    }

    uiInterface.NotifyBlocksChanged();
    return true;
}



int BlockHeight(uint256 bh)
{
	int nGridHeight=0;
	map<uint256, CBlockIndex*>::iterator iMapIndex = mapBlockIndex.find(bh);
    if (iMapIndex != mapBlockIndex.end())
	{
		 CBlockIndex* pPrev = (*iMapIndex).second;
		 nGridHeight = pPrev->nHeight+1;
	}
	return nGridHeight;
}


bool CBlock::CheckBlock(std::string sCaller, int height1, int64_t Mint, bool fCheckPOW, bool fCheckMerkleRoot, bool fCheckSig, bool fLoadingIndex) const
{

	if (GetHash()==hashGenesisBlock || GetHash()==hashGenesisBlockTestNet) return true;
	// These are checks that are independent of context
    // that can be verified before saving an orphan block.

    // Size limits
    if (vtx.empty() || vtx.size() > MAX_BLOCK_SIZE || ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION) > MAX_BLOCK_SIZE)
        return DoS(100, error("CheckBlock[] : size limits failed"));

    // Check proof of work matches claimed amount
    if (fCheckPOW && IsProofOfWork() && !CheckProofOfWork(GetPoWHash(), nBits))
        return DoS(50, error("CheckBlock[] : proof of work failed"));

	//Reject blocks with diff that has grown to an extrordinary level (should never happen)
	double blockdiff = GetBlockDifficulty(nBits);
	if (height1 > nGrandfather && blockdiff > 10000000000000000)
	{
  	   return DoS(1, error("CheckBlock[] : Block Bits larger than 10000000000000000.\r\n"));
	}

    // First transaction must be coinbase, the rest must not be
    if (vtx.empty() || !vtx[0].IsCoinBase())
        return DoS(100, error("CheckBlock[] : first tx is not coinbase"));
    for (unsigned int i = 1; i < vtx.size(); i++)
        if (vtx[i].IsCoinBase())
            return DoS(100, error("CheckBlock[] : more than one coinbase"));

	//Research Age
	MiningCPID bb = DeserializeBoincBlock(vtx[0].hashBoinc);
	//For higher security, plus lets catch these bad blocks before adding them to the chain to prevent reorgs:
	double OUT_POR = 0;
	double OUT_INTEREST = 0;
	double dAccrualAge = 0;
	double dMagnitudeUnit = 0;
	double dAvgMagnitude = 0;
	int64_t nCoinAge = 0;
	int64_t nFees = 0;

	if (bb.cpid != "INVESTOR" && IsProofOfStake() && height1 > nGrandfather && IsResearchAgeEnabled(height1) && BlockNeedsChecked(nTime) && !fLoadingIndex)
	{
			int64_t nCalculatedResearch = GetProofOfStakeReward(nCoinAge, nFees, bb.cpid, true, 1, nTime,
				pindexBest, sCaller + "_checkblock_researcher", OUT_POR, OUT_INTEREST, dAccrualAge, dMagnitudeUnit, dAvgMagnitude);
			if (bb.ResearchSubsidy > ((OUT_POR*1.25)+1))
			{
				BusyWaitForTally();
				StructCPID st1 = GetLifetimeCPID(bb.cpid,"CheckBlock()");
				nCalculatedResearch = GetProofOfStakeReward(nCoinAge, nFees, bb.cpid, true, 2, nTime,
					pindexBest, sCaller + "_checkblock_researcher_doublecheck", OUT_POR, OUT_INTEREST, dAccrualAge, dMagnitudeUnit, dAvgMagnitude);

				if (bb.ResearchSubsidy > ((OUT_POR*1.25)+1))
				{

							if (fDebug3) printf("CheckBlock[ResearchAge] : Researchers Reward Pays too much : Interest %f and Research %f and StakeReward %f, OUT_POR %f, with Out_Interest %f for CPID %s ",
									(double)bb.InterestSubsidy,(double)bb.ResearchSubsidy,CoinToDouble(nCalculatedResearch),(double)OUT_POR,(double)OUT_INTEREST,bb.cpid.c_str());
	
							return DoS(10,error("CheckBlock[ResearchAge] : Researchers Reward Pays too much : Interest %f and Research %f and StakeReward %f, OUT_POR %f, with Out_Interest %f for CPID %s ",
									(double)bb.InterestSubsidy,(double)bb.ResearchSubsidy,CoinToDouble(nCalculatedResearch),(double)OUT_POR,(double)OUT_INTEREST,bb.cpid.c_str()));
							// Reserved for future use.
				}
			}
	
	}


	//ProofOfResearch
	if (vtx.size() > 0)
	{
	//Orphan Flood Attack
			if (height1 > nGrandfather)
			{
					double bv = BlockVersion(bb.clientversion);
					double cvn = ClientVersionNew();
					if (fDebug10) printf("BV %f, CV %f   ",bv,cvn);
					// if (bv+10 < cvn) return error("ConnectBlock[]: Old client version after mandatory upgrade - block rejected\r\n");
					// Enforce Beacon Age
					if (bv < 3588 && height1 > 860500 && !fTestNet) return error("CheckBlock[]:  Old client spamming new blocks after mandatory upgrade \r\n");
					if (bv < 3580 && fTestNet) return DoS(25, error("CheckBlock[]:  Old testnet client spamming new blocks after mandatory upgrade \r\n"));
			}

			if (bb.cpid != "INVESTOR" && height1 > nGrandfather && BlockNeedsChecked(nTime))
			{
    			if (bb.projectname.empty() && !IsResearchAgeEnabled(height1)) 	return DoS(1,error("CheckBlock::PoR Project Name invalid"));
	    		if (!fLoadingIndex && !IsCPIDValidv2(bb,height1))
				{
						std::string sOut2 = "";
						LoadAdminMessages(false,sOut2);
						if (!fLoadingIndex && !IsCPIDValidv2(bb,height1))
						{
							return error("Bad CPID : height %f, CPID %s, cpidv2 %s, LBH %s, Bad Hashboinc %s",(double)height1,
								bb.cpid.c_str(), bb.cpidv2.c_str(),
								bb.lastblockhash.c_str(), vtx[0].hashBoinc.c_str());
						}
				}

			}

		    // Gridcoin: check proof-of-stake block signature
			if (IsProofOfStake() && height1 > nGrandfather)
			{
				//Mint limiter checks 1-20-2015
				double PORDiff = GetBlockDifficulty(nBits);
				double mint1 = CoinToDouble(Mint);
				double total_subsidy = bb.ResearchSubsidy + bb.InterestSubsidy;
				if (fDebug10) printf("CheckBlock[]: TotalSubsidy %f, Height %f, %s, %f, Res %f, Interest %f, hb: %s \r\n",
					    (double)total_subsidy,(double)height1, bb.cpid.c_str(),
						(double)mint1,bb.ResearchSubsidy,bb.InterestSubsidy,vtx[0].hashBoinc.c_str());
				if (total_subsidy < MintLimiter(PORDiff,bb.RSAWeight,bb.cpid,GetBlockTime()))
				{
					if (fDebug3) printf("****CheckBlock[]: Total Mint too Small %s, mint %f, Res %f, Interest %f, hash %s \r\n",bb.cpid.c_str(),
						(double)mint1,bb.ResearchSubsidy,bb.InterestSubsidy,vtx[0].hashBoinc.c_str());
					//1-21-2015 - Prevent Hackers from spamming the network with small blocks
					return error("****CheckBlock[]: Total Mint too Small %s, mint %f, Res %f, Interest %f, hash %s \r\n",bb.cpid.c_str(),
							(double)mint1,bb.ResearchSubsidy,bb.InterestSubsidy,vtx[0].hashBoinc.c_str());
				}

	    		if (fCheckSig && !CheckBlockSignature())
					return DoS(100, error("CheckBlock[] : bad proof-of-stake block signature"));
			}


		}
		else
		{
			return false;
		}

	// End of Proof Of Research

    if (IsProofOfStake())
    {
	    // Coinbase output should be empty if proof-of-stake block
        if (vtx[0].vout.size() != 1 || !vtx[0].vout[0].IsEmpty())
            return DoS(100, error("CheckBlock[] : coinbase output not empty for proof-of-stake block"));

        // Second transaction must be coinstake, the rest must not be
        if (vtx.empty() || !vtx[1].IsCoinStake())
            return DoS(100, error("CheckBlock[] : second tx is not coinstake"));

	    for (unsigned int i = 2; i < vtx.size(); i++)
		{
            if (vtx[i].IsCoinStake())
			{
				printf("Found more than one coinstake in coinbase at location %f\r\n",(double)i);
                return DoS(100, error("CheckBlock[] : more than one coinstake"));
			}
		}

    }

    // Check transactions
    BOOST_FOREACH(const CTransaction& tx, vtx)
    {
        if (!tx.CheckTransaction())
            return DoS(tx.nDoS, error("CheckBlock[] : CheckTransaction failed"));

        // ppcoin: check transaction timestamp
        if (GetBlockTime() < (int64_t)tx.nTime)
            return DoS(50, error("CheckBlock[] : block timestamp earlier than transaction timestamp"));
    }

    // Check for duplicate txids. This is caught by ConnectInputs(),
    // but catching it earlier avoids a potential DoS attack:
    set<uint256> uniqueTx;
    BOOST_FOREACH(const CTransaction& tx, vtx)
    {
        uniqueTx.insert(tx.GetHash());
    }
    if (uniqueTx.size() != vtx.size())
        return DoS(100, error("CheckBlock[] : duplicate transaction"));

    unsigned int nSigOps = 0;
    BOOST_FOREACH(const CTransaction& tx, vtx)
    {
        nSigOps += tx.GetLegacySigOpCount();
    }
    if (nSigOps > MAX_BLOCK_SIGOPS)
        return DoS(100, error("CheckBlock[] : out-of-bounds SigOpCount"));

    // Check merkle root
    if (fCheckMerkleRoot && hashMerkleRoot != BuildMerkleTree())
        return DoS(100, error("CheckBlock[] : hashMerkleRoot mismatch"));

	//if (fDebug3) printf(".EOCB.");
    return true;
}

bool CBlock::AcceptBlock(bool generated_by_me)
{
    AssertLockHeld(cs_main);

    if (nVersion > CURRENT_VERSION)
        return DoS(100, error("AcceptBlock() : reject unknown block version %d", nVersion));

    // Check for duplicate
    uint256 hash = GetHash();
    if (mapBlockIndex.count(hash))
        return error("AcceptBlock() : block already in mapBlockIndex");

    // Get prev block index
    map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashPrevBlock);
    if (mi == mapBlockIndex.end())
        return DoS(10, error("AcceptBlock() : prev block not found"));
    CBlockIndex* pindexPrev = (*mi).second;
    int nHeight = pindexPrev->nHeight+1;

    if (IsProtocolV2(nHeight) && nVersion < 7)
        return DoS(100, error("AcceptBlock() : reject too old nVersion = %d", nVersion));
    else if (!IsProtocolV2(nHeight) && nVersion > 6)
        return DoS(100, error("AcceptBlock() : reject too new nVersion = %d", nVersion));

    if (IsProofOfWork() && nHeight > LAST_POW_BLOCK)
        return DoS(100, error("AcceptBlock() : reject proof-of-work at height %d", nHeight));

	if (nHeight > nGrandfather)
	{
			// Check coinbase timestamp
			if (GetBlockTime() > FutureDrift((int64_t)vtx[0].nTime, nHeight))
			{
				return DoS(80, error("AcceptBlock() : coinbase timestamp is too early"));
			}
			// Check timestamp against prev
			if (GetBlockTime() <= pindexPrev->GetPastTimeLimit() || FutureDrift(GetBlockTime(), nHeight) < pindexPrev->GetBlockTime())
				return DoS(60, error("AcceptBlock() : block's timestamp is too early"));
    		// Check proof-of-work or proof-of-stake
	    	if (nBits != GetNextTargetRequired(pindexPrev, IsProofOfStake()))
				return DoS(100, error("AcceptBlock() : incorrect %s", IsProofOfWork() ? "proof-of-work" : "proof-of-stake"));
	}


    // Check that all transactions are finalized
    BOOST_FOREACH(const CTransaction& tx, vtx)
        if (!IsFinalTx(tx, nHeight, GetBlockTime()))
            return DoS(10, error("AcceptBlock() : contains a non-final transaction"));

    // Check that the block chain matches the known block chain up to a checkpoint
    if (!Checkpoints::CheckHardened(nHeight, hash))
        return DoS(100, error("AcceptBlock() : rejected by hardened checkpoint lock-in at %d", nHeight));

    uint256 hashProof;

    // Verify hash target and signature of coinstake tx
	if (nHeight > nGrandfather)
	{
				if (IsProofOfStake())
				{
					uint256 targetProofOfStake;
					if (!CheckProofOfStake(pindexPrev, vtx[1], nBits, hashProof, targetProofOfStake, vtx[0].hashBoinc, generated_by_me, nNonce) && IsLockTimeWithinMinutes(GetBlockTime(),600))
					{
						return error("WARNING: AcceptBlock(): check proof-of-stake failed for block %s, nonce %f    \n", hash.ToString().c_str(),(double)nNonce);
					}

				}
	}


    // PoW is checked in CheckBlock[]
    if (IsProofOfWork())
    {
        hashProof = GetPoWHash();
    }

	//Grandfather
	if (nHeight > nGrandfather)
	{
		 bool cpSatisfies = Checkpoints::CheckSync(hash, pindexPrev);
 		 // Check that the block satisfies synchronized checkpoint
		 if (CheckpointsMode == Checkpoints::STRICT && !cpSatisfies)
		 {
			if (CHECKPOINT_DISTRIBUTED_MODE==1)
			{
				CHECKPOINT_VIOLATIONS++;
				if (CHECKPOINT_VIOLATIONS > 3)
				{
					//For stability, move the client into ADVISORY MODE:
					printf("Moving Gridcoin into Checkpoint ADVISORY mode.\r\n");
					CheckpointsMode = Checkpoints::ADVISORY;
				}
			}
			return error("AcceptBlock() : rejected by synchronized checkpoint");
		 }

		if (CheckpointsMode == Checkpoints::ADVISORY && !cpSatisfies)
			strMiscWarning = _("WARNING: synchronized checkpoint violation detected, but skipped!");

		if (CheckpointsMode == Checkpoints::ADVISORY && cpSatisfies && CHECKPOINT_DISTRIBUTED_MODE==1)
		{
			///Move the client back into STRICT mode
			CHECKPOINT_VIOLATIONS = 0;
			printf("Moving Gridcoin into Checkpoint STRICT mode.\r\n");
			strMiscWarning = "";
			CheckpointsMode = Checkpoints::STRICT;
		}

		// Enforce rule that the coinbase starts with serialized block height
		CScript expect = CScript() << nHeight;
		if (vtx[0].vin[0].scriptSig.size() < expect.size() ||
			!std::equal(expect.begin(), expect.end(), vtx[0].vin[0].scriptSig.begin()))
			return DoS(100, error("AcceptBlock() : block height mismatch in coinbase"));
	}

    // Write block to history file
    if (!CheckDiskSpace(::GetSerializeSize(*this, SER_DISK, CLIENT_VERSION)))
        return error("AcceptBlock() : out of disk space");
    unsigned int nFile = -1;
    unsigned int nBlockPos = 0;
    if (!WriteToDisk(nFile, nBlockPos))
        return error("AcceptBlock() : WriteToDisk failed");
    if (!AddToBlockIndex(nFile, nBlockPos, hashProof))
        return error("AcceptBlock() : AddToBlockIndex failed");

    // Relay inventory, but don't relay old inventory during initial block download
    int nBlockEstimate = Checkpoints::GetTotalBlocksEstimate();
    if (hashBestChain == hash)
    {
        LOCK(cs_vNodes);
        BOOST_FOREACH(CNode* pnode, vNodes)
            if (nBestHeight > (pnode->nStartingHeight != -1 ? pnode->nStartingHeight - 2000 : nBlockEstimate))
                pnode->PushInventory(CInv(MSG_BLOCK, hash));
    }

    // ppcoin: check pending sync-checkpoint
    Checkpoints::AcceptPendingSyncCheckpoint();
	if (fDebug) printf("{ACC}");
	nLastAskedForBlocks=GetAdjustedTime();
	ResetTimerMain("OrphanBarrage");
    return true;
}


uint256 CBlockIndex::GetBlockTrust() const
{
    CBigNum bnTarget;
    bnTarget.SetCompact(nBits);
    if (bnTarget <= 0) return 0;
	int64_t block_mag = 0;
	uint256 chaintrust = (((CBigNum(1)<<256) / (bnTarget+1)) - (block_mag)).getuint256();
	return chaintrust;
}

bool CBlockIndex::IsSuperMajority(int minVersion, const CBlockIndex* pstart, unsigned int nRequired, unsigned int nToCheck)
{
    unsigned int nFound = 0;
    for (unsigned int i = 0; i < nToCheck && nFound < nRequired && pstart != NULL; i++)
    {
        if (pstart->nVersion >= minVersion)
            ++nFound;
        pstart = pstart->pprev;
    }
    return (nFound >= nRequired);
}

/*
bool static ReserealizeBlockSignature(CBlock* pblock)
{
    if (pblock->IsProofOfWork()) {
        pblock->vchBlockSig.clear();
        return true;
    }

    return CKey::ReserealizeSignature(pblock->vchBlockSig);
}
*/


bool ServicesIncludesNN(CNode* pNode)
{
	return (Contains(pNode->strSubVer,"1999")) ? true : false;
}

bool VerifySuperblock(std::string superblock, int nHeight)
{
	    bool bPassed = false;
		double out_avg = 0;
		double out_beacon_count=0;
		double out_participant_count=0;
		double avg_mag = 0;
		if (superblock.length() > 20)
		{
			avg_mag = GetSuperblockAvgMag(superblock,out_beacon_count,out_participant_count,out_avg,false);
			bPassed=true;
			if (!IsResearchAgeEnabled(nHeight))
			{
				return (avg_mag < 10 ? false : true);
			}
			// New rules added here:
			if (out_avg < 10 && fTestNet)  bPassed = false;
			if (out_avg < 70 && !fTestNet) bPassed = false;
			if (avg_mag < 10)              bPassed = false;
		}
		if (fDebug3 && !bPassed)
		{
			if (fDebug) printf(" Verification of Superblock Failed ");
			//			printf("\r\n Verification of Superblock Failed outavg: %f, avg_mag %f, Height %f, Out_Beacon_count %f, Out_participant_count %f, block %s",	(double)out_avg,(double)avg_mag,(double)nHeight,(double)out_beacon_count,(double)out_participant_count,superblock.c_str());
		}
		return bPassed;
}

bool NeedASuperblock()
{
		bool bDireNeedOfSuperblock = false;
		std::string superblock = ReadCache("superblock","all");
		if (superblock.length() > 20 && !OutOfSyncByAge())
		{
			if (!VerifySuperblock(superblock,pindexBest->nHeight)) bDireNeedOfSuperblock = true;
		}
		int64_t superblock_age = GetAdjustedTime() - mvApplicationCacheTimestamp["superblock;magnitudes"];
		if ((double)superblock_age > (double)(GetSuperblockAgeSpacing(nBestHeight))) bDireNeedOfSuperblock = true;
		return bDireNeedOfSuperblock;
}




void GridcoinServices()
{

	//Dont do this on headless - SeP
	#if defined(QT_GUI)
	   if ((nBestHeight % 125) == 0)
	   {
			GetGlobalStatus();
			bForceUpdate=true;
			uiInterface.NotifyBlocksChanged();
	   }
    #endif
	// Services thread activity
    
	//This is Gridcoins Service thread; called once per block
	if (nBestHeight > 100 && nBestHeight < 200)
	{
		if (GetArg("-suppressdownloadblocks", "true") == "false")
		{
			std::string email = GetArgument("email", "NA");
			if (email.length() > 5 && !mbBlocksDownloaded)
			{
				#if defined(WIN32) && defined(QT_GUI)
					mbBlocksDownloaded=true;
					DownloadBlocks();
				#endif
			}
		}
	}
	//Dont perform the following functions if out of sync
	if (pindexBest->nHeight < nGrandfather) return;
    
	if (OutOfSyncByAge()) return;
	if (fDebug) printf(" {SVC} ");

	//Backup the wallet once per 900 blocks:
	double dWBI = cdbl(GetArgument("walletbackupinterval", "900"),0);
	
	if (TimerMain("backupwallet", dWBI))
	{
		std::string backup_results = BackupGridcoinWallet();
		printf("Daily backup results: %s\r\n",backup_results.c_str());
	}

	if (TimerMain("ResetVars",30))
	{
		bTallyStarted = false;
	}
	
	if (TimerMain("OutOfSyncDaily",900))
	{
		if (WalletOutOfSync())
		{
			printf("Restarting Gridcoin...");
			#if defined(WIN32) && defined(QT_GUI)
				int iResult = RestartClient();
			#endif
		}
	}

	if (false && TimerMain("FixSpentCoins",60))
	{
			int nMismatchSpent;
			int64_t nBalanceInQuestion;
			pwalletMain->FixSpentCoins(nMismatchSpent, nBalanceInQuestion);
    }

	if (TimerMain("MyNeuralMagnitudeReport",30))
	{
		try
		{
			if (msNeuralResponse.length() < 25 && msPrimaryCPID != "INVESTOR" && !msPrimaryCPID.empty())
			{
				AsyncNeuralRequest("explainmag",msPrimaryCPID,5);
				if (fDebug3) printf("Async explainmag sent for %s.",msPrimaryCPID.c_str());
			}
			// Run the RSA report for the overview page:
		    if (!msPrimaryCPID.empty() && msPrimaryCPID != "INVESTOR")
			{
				if (fDebug3) printf("updating rsa\r\n");
				MagnitudeReport(msPrimaryCPID);
				if (fDebug3) printf("updated rsa\r\n");
			}
			if (fDebug3) printf("\r\n MR Complete \r\n");
		}
		catch (std::exception &e)
		{
			printf("Error in MyNeuralMagnitudeReport1.");
		}
		catch(...)
		{
			printf("Error in MyNeuralMagnitudeReport.");
		}
	}

	int64_t superblock_age = GetAdjustedTime() - mvApplicationCacheTimestamp["superblock;magnitudes"];
	bool bNeedSuperblock = ((double)superblock_age > (double)(GetSuperblockAgeSpacing(nBestHeight)));
	if ( nBestHeight % 3 == 0 && NeedASuperblock() ) bNeedSuperblock=true;

	if (fDebug10) 
	{
			printf (" MRSA %f, BH %f ",(double)superblock_age,(double)nBestHeight);
	}

	if (bNeedSuperblock)
	{
		if ((nBestHeight % 3) == 0)
		{
			if (fDebug3) printf("#CNNSH# ");
			ComputeNeuralNetworkSupermajorityHashes();
			UpdateNeuralNetworkQuorumData();
		}
		if ((nBestHeight % 20) == 0)
		{
			if (fDebug3) printf("#TIB# ");
			bDoTally = true;
		}
	}
	else
	{
		// When superblock is not old, Tally every N mins:
		int nTallyGranularity = fTestNet ? 60 : 20;
		if ((nBestHeight % nTallyGranularity) == 0)
		{
			    if (fDebug3) printf("TIB1 ");
			    bDoTally = true;
				if (fDebug3) printf("CNNSH2 ");
				ComputeNeuralNetworkSupermajorityHashes();
		}

		if ((nBestHeight % 5)==0)
		{
				UpdateNeuralNetworkQuorumData();
		}

	}

	// Keep Local Neural Network in Sync once every 1/2 day
	if (TimerMain("SyncNeuralNetwork",500))
	{
		FullSyncWithDPORNodes();
	}


	// Every N blocks as a Synchronized TEAM:
	if ((nBestHeight % 30) == 0)
	{
		//Sync RAC with neural network IF superblock is over 24 hours Old, Or if we have No superblock (in case of the latter, age will be 45 years old)
		// Note that nodes will NOT accept superblocks without a supermajority hash, so the last block will not be in memory unless it is a good superblock.
		// Let's start syncing the neural network as soon as the LAST superblock is over 12 hours old.
		// Also, lets do this as a TEAM exactly every 30 blocks (~30 minutes) to try to reach an EXACT consensus every half hour:
		// For effeciency, the network sleeps for 20 hours after a good superblock is accepted
		if (NeedASuperblock() && NeuralNodeParticipates())
		{
			if (fDebug3) printf("FSWDPOR ");
			FullSyncWithDPORNodes();
		}
	}

	if (( (nBestHeight-10) % 30 ) == 0)
	{
			// 10 Blocks after the network started syncing the neural network as a team, ask the neural network to come to a quorum
			if (NeedASuperblock() && NeuralNodeParticipates())
			{
				// First verify my node has a synced contract
				std::string contract = "";
				#if defined(WIN32) && defined(QT_GUI)
					contract = qtGetNeuralContract("");
				#endif
				if (VerifySuperblock(contract,nBestHeight))
				{
						AsyncNeuralRequest("quorum","gridcoin",25);
				}
			}
	}


	if (TimerMain("send_beacon",180))
	{
		std::string sOutPubKey = "";
		std::string sOutPrivKey = "";
		std::string sError = "";
		std::string sMessage = "";
		bool fResult = AdvertiseBeacon(true,sOutPrivKey,sOutPubKey,sError,sMessage);
		if (!fResult)
		{
			printf("BEACON ERROR!  Unable to send beacon %s \r\n",sError.c_str());
			printf("BEACON ERROR!  Unable to send beacon %s \r\n",sMessage.c_str());
			msMiningErrors6 = "Unable To Send Beacon! Unlock Wallet!";
		}
	}

	if (false && TimerMain("GridcoinPersistedDataSystem",5))
	{
		std::string errors1 = "";
		LoadAdminMessages(false,errors1);
	}

	if (KeyEnabled("exportmagnitude"))
	{
		if (TimerMain("export_magnitude",900))
		{
			json_spirit::Array results;
		    results = MagnitudeReportCSV(true);

		}
	}

	if (TimerMain("gather_cpids",480))
	{
			//if (fDebug10) printf("\r\nReharvesting cpids in background thread...\r\n");
			//LoadCPIDsInBackground();
			//printf(" {CPIDs Re-Loaded} ");
			msNeuralResponse="";
	}

	if (TimerMain("clearcache",1000))
	{
		ClearCache("neural_data");
	}

	if (TimerMain("check_for_autoupgrade",240))
	{
		if (fDebug3) printf("Checking for upgrade...");
		bCheckedForUpgradeLive = true;
	}

	#if defined(WIN32) && defined(QT_GUI)
		if (bCheckedForUpgradeLive && !fTestNet && bProjectsInitialized && bGlobalcomInitialized)
		{
			bCheckedForUpgradeLive=false;
			printf("{Checking for Upgrade} ");
			CheckForUpgrade();
			printf("{Done checking for upgrade} ");
		}
	#endif
    if (fDebug10) printf(" {/SVC} ");

}



bool AskForOutstandingBlocks(uint256 hashStart)
{
	if (IsLockTimeWithinMinutes(nLastAskedForBlocks,2)) return true;
	nLastAskedForBlocks = GetAdjustedTime();
		
	int iAsked = 0;
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pNode, vNodes) 
	{
				pNode->ClearBanned();
    			if (!pNode->fClient && !pNode->fOneShot && (pNode->nStartingHeight > (nBestHeight - 144)) && (pNode->nVersion < NOBLKS_VERSION_START || pNode->nVersion >= NOBLKS_VERSION_END) )
				{
					    if (hashStart==uint256(0))
						{
							pNode->PushGetBlocks(pindexBest, uint256(0), true);
						}
						else
						{
							CBlockIndex* pblockindex = mapBlockIndex[hashStart];
							if (pblockindex)
							{
	    						pNode->PushGetBlocks(pblockindex, uint256(0), true);
							}
							else
							{
								return error("Unable to find block index %s",hashStart.ToString().c_str());
							}
						}
						printf(".B.");
						iAsked++;
						if (iAsked > 10) break;
				}
	}
	return true;
}





void CheckForLatestBlocks()
{
	if (WalletOutOfSync())
	{
			mapOrphanBlocks.clear();
			setStakeSeen.clear();
			setStakeSeenOrphan.clear();
			bool fResult = AskForOutstandingBlocks(uint256(0));
			printf("\r\n ** Clearing Orphan Blocks... ** \r\n");
	}
	
}

void CleanInboundConnections(bool bClearAll)
{
		if (IsLockTimeWithinMinutes(nLastCleaned,10)) return;
     	nLastCleaned = GetAdjustedTime();
	 	LOCK(cs_vNodes);
		BOOST_FOREACH(CNode* pNode, vNodes) 
		{
				pNode->ClearBanned();
				if (pNode->nStartingHeight < (nBestHeight-1000) || bClearAll)
				{
						pNode->fDisconnect=true;
				}
		}
		printf("\r\n Cleaning inbound connections \r\n");
}


bool WalletOutOfSync()
{
	// Only trigger an out of sync condition if the node has synced near the best block prior to going out of sync.
	bool fOut = OutOfSyncByMoreThan(30);
	double PORDiff = GetDifficulty(GetLastBlockIndex(pindexBest, true));
	bool fGhostChain = (!fTestNet && PORDiff < .75);
	int iPeerBlocks = GetNumBlocksOfPeers();
	bool bSyncedCloseToTop = nBestHeight > iPeerBlocks-1000;
	if ((fOut || fGhostChain) && bSyncedCloseToTop) return true;
	return false;
}


bool WalletOutOfSyncByMoreThan2000Blocks()
{
	if (nBestHeight < GetNumBlocksOfPeers()-2000) return true;
	return false;
}



void CheckForFutileSync()
{
	// If we stay out of sync for more than 8 iterations of 25 orphans and never recover without accepting a block - attempt to recover the node- if we recover, reset the counters.
	// We reset these counters every time a block is accepted successfully in AcceptBlock().
	// Note: This code will never actually be exercised unless the wallet stays out of sync for a very long time - approx. 24 hours - the wallet normally recovers on its own without this code.
	// I'm leaving this in for people who may be on vacation for a long time - it may keep an external node running when everything else fails.
	if (WalletOutOfSync())
	{
		if (TimerMain("CheckForFutileSync", 25))
		{
			if (TimerMain("OrphansAndNotRecovering",8))									
			{
				printf("\r\nGridcoin has not recovered after clearing orphans; Restarting node...\r\n");
				#if defined(WIN32) && defined(QT_GUI)
					int iResult = RestartClient();
				#endif
			}
			else
			{
				mapAlreadyAskedFor.clear();
				printf("\r\nClearing mapAlreadyAskedFor.\r\n");
				mapOrphanBlocks.clear(); 
				setStakeSeen.clear();  
				setStakeSeenOrphan.clear();
				AskForOutstandingBlocks(uint256(0));
			}
		}
		else
		{
			ResetTimerMain("OrphansAndNotRecovering");
		}
	}
}

bool ProcessBlock(CNode* pfrom, CBlock* pblock, bool generated_by_me)
{
    AssertLockHeld(cs_main);

    // Check for duplicate
    uint256 hash = pblock->GetHash();
    if (mapBlockIndex.count(hash))
        return error("ProcessBlock() : already have block %d %s", mapBlockIndex[hash]->nHeight, hash.ToString().c_str());
    if (mapOrphanBlocks.count(hash))
        return error("ProcessBlock() : already have block (orphan) %s", hash.ToString().c_str());

    // ppcoin: check proof-of-stake
    // Limited duplicity on stake: prevents block flood attack
    // Duplicate stake allowed only when there is orphan child block
    if (pblock->IsProofOfStake() && setStakeSeen.count(pblock->GetProofOfStake()) && !mapOrphanBlocksByPrev.count(hash) && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
        return error("ProcessBlock() : duplicate proof-of-stake (%s, %d) for block %s", pblock->GetProofOfStake().first.ToString().c_str(),
		pblock->GetProofOfStake().second, 
		hash.ToString().c_str());

    CBlockIndex* pcheckpoint = Checkpoints::GetLastSyncCheckpoint();
    if (pcheckpoint && pblock->hashPrevBlock != hashBestChain && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
    {
        // Extra checks to prevent "fill up memory by spamming with bogus blocks"
        int64_t deltaTime = pblock->GetBlockTime() - pcheckpoint->nTime;
        if (deltaTime < -10*60)
        {
            if (pfrom)
                pfrom->Misbehaving(1);
            return error("ProcessBlock() : block with timestamp before last checkpoint");
        }


    }

    // Preliminary checks
    if (!pblock->CheckBlock("ProcessBlock", pindexBest->nHeight, 100*COIN))
        return error("ProcessBlock() : CheckBlock FAILED");

    // ppcoin: ask for pending sync-checkpoint if any
    if (!IsInitialBlockDownload())
        Checkpoints::AskForPendingSyncCheckpoint(pfrom);


    // If don't already have its previous block, shunt it off to holding area until we get it
    if (!mapBlockIndex.count(pblock->hashPrevBlock))
    {
		// *****      This area covers Gridcoin Orphan Handling      ***** 
		if (true)
		{
			if (WalletOutOfSync())
			{
				if (TimerMain("OrphanBarrage",100))
				{
					mapAlreadyAskedFor.clear();
					printf("\r\nClearing mapAlreadyAskedFor.\r\n");
					AskForOutstandingBlocks(uint256(0));
					CheckForFutileSync();
				}
			}
		}

		CBlock* pblock2 = new CBlock(*pblock);
		if (WalletOutOfSyncByMoreThan2000Blocks() || fTestNet)
		{
			printf("ProcessBlock: ORPHAN BLOCK, prev=%s\n", pblock->hashPrevBlock.ToString().c_str());
			// ppcoin: check proof-of-stake
			if (pblock->IsProofOfStake())
			{
					// Limited duplicity on stake: prevents block flood attack
					// Duplicate stake allowed only when there is orphan child block
    				if (setStakeSeenOrphan.count(pblock->GetProofOfStake()) && !mapOrphanBlocksByPrev.count(hash) && !Checkpoints::WantedByPendingSyncCheckpoint(hash))
							return error("ProcessBlock() : duplicate proof-of-stake (%s, %d) for orphan block %s", pblock->GetProofOfStake().first.ToString().c_str(), pblock->GetProofOfStake().second, hash.ToString().c_str());
						else
							setStakeSeenOrphan.insert(pblock->GetProofOfStake());
			}
			mapOrphanBlocks.insert(make_pair(hash, pblock2));
			mapOrphanBlocksByPrev.insert(make_pair(pblock2->hashPrevBlock, pblock2));
		}

		// Ask this guy to fill in what we're missing
        if (pfrom)
        {
            pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(pblock2), true);
            // ppcoin: getblocks may not obtain the ancestor block rejected
            // earlier by duplicate-stake check so we ask for it again directly
            if (!IsInitialBlockDownload())
                pfrom->AskFor(CInv(MSG_BLOCK, WantedByOrphan(pblock2)));
			// Ask a few other nodes for the missing block

        }
        return true;
    }

    // Store to disk
    if (!pblock->AcceptBlock(generated_by_me))
        return error("ProcessBlock() : AcceptBlock FAILED");

    // Recursively process any orphan blocks that depended on this one
    vector<uint256> vWorkQueue;
    vWorkQueue.push_back(hash);
    for (unsigned int i = 0; i < vWorkQueue.size(); i++)
    {
        uint256 hashPrev = vWorkQueue[i];
        for (multimap<uint256, CBlock*>::iterator mi = mapOrphanBlocksByPrev.lower_bound(hashPrev);
             mi != mapOrphanBlocksByPrev.upper_bound(hashPrev);
             ++mi)
        {
            CBlock* pblockOrphan = (*mi).second;
            if (pblockOrphan->AcceptBlock(generated_by_me))
                vWorkQueue.push_back(pblockOrphan->GetHash());
            mapOrphanBlocks.erase(pblockOrphan->GetHash());
            setStakeSeenOrphan.erase(pblockOrphan->GetProofOfStake());
            delete pblockOrphan;
        }
        mapOrphanBlocksByPrev.erase(hashPrev);
    }

   
    // if responsible for sync-checkpoint send it
    if (false && pfrom && !CSyncCheckpoint::strMasterPrivKey.empty())        Checkpoints::SendSyncCheckpoint(Checkpoints::AutoSelectSyncCheckpoint());
	printf("{PB}: ACC; \r\n");
	GridcoinServices();
    return true;
}



// Gridcoin: (previously NovaCoin) : Attempt to generate suitable proof-of-stake
bool CBlock::SignBlock(CWallet& wallet, int64_t nFees)
{
    // if we are trying to sign
    //    something except proof-of-stake block template
    if (!vtx[0].vout[0].IsEmpty())
        return false;

    // if we are trying to sign
    //    a complete proof-of-stake block
    if (IsProofOfStake())
        return true;

    static int64_t nLastCoinStakeSearchTime = GetAdjustedTime(); // startup timestamp

    CKey key;
    CTransaction txCoinStake;
    if (IsProtocolV2(nBestHeight+1))
        txCoinStake.nTime &= ~STAKE_TIMESTAMP_MASK;

    int64_t nSearchTime = txCoinStake.nTime; // search to current time
	int64_t out_gridreward = 0;

    if (nSearchTime > nLastCoinStakeSearchTime)
    {
        int64_t nSearchInterval = IsProtocolV2(nBestHeight+1) ? 1 : nSearchTime - nLastCoinStakeSearchTime;
		std::string out_hashboinc = "";
        if (wallet.CreateCoinStake(wallet, nBits, nSearchInterval, nFees, txCoinStake, key, out_gridreward, out_hashboinc))
        {
			//1-8-2015 Extract solved Key
			double solvedNonce = cdbl(AppCache(pindexBest->GetBlockHash().GetHex()),0);
			nNonce=solvedNonce;
			if (fDebug3) printf(".17. Nonce %f, SNonce %f, StakeTime %f, MaxHistTD %f, BBPTL %f, PDBTm %f \r\n",
				(double)nNonce,(double)solvedNonce,(double)txCoinStake.nTime,
				(double)max(pindexBest->GetPastTimeLimit()+1, PastDrift(pindexBest->GetBlockTime(), pindexBest->nHeight+1)),
				(double)pindexBest->GetPastTimeLimit(), (double)PastDrift(pindexBest->GetBlockTime(), pindexBest->nHeight+1)	);
		    if (txCoinStake.nTime >= max(pindexBest->GetPastTimeLimit()+1, PastDrift(pindexBest->GetBlockTime(), pindexBest->nHeight+1)))
			{
                // make sure coinstake would meet timestamp protocol
                //    as it would be the same as the block timestamp
                vtx[0].nTime = nTime = txCoinStake.nTime;
                nTime = max(pindexBest->GetPastTimeLimit()+1, GetMaxTransactionTime());
                nTime = max(GetBlockTime(), PastDrift(pindexBest->GetBlockTime(), pindexBest->nHeight+1));


                // we have to make sure that we have no future timestamps in
                //    our transactions set
                for (vector<CTransaction>::iterator it = vtx.begin(); it != vtx.end();)
                    if (it->nTime > nTime) { it = vtx.erase(it); } else { ++it; }
                vtx.insert(vtx.begin() + 1, txCoinStake);
				vtx[0].hashBoinc= out_hashboinc;

                hashMerkleRoot = BuildMerkleTree();
				return key.Sign(GetHash(), vchBlockSig);
			}
        }
        nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
        nLastCoinStakeSearchTime = nSearchTime;
    }

    return false;
}

bool CBlock::CheckBlockSignature() const
{
    if (IsProofOfWork())
        return vchBlockSig.empty();

    vector<valtype> vSolutions;
    txnouttype whichType;

    const CTxOut& txout = vtx[1].vout[1];

    if (!Solver(txout.scriptPubKey, whichType, vSolutions))
        return false;

    if (whichType == TX_PUBKEY)
    {
        valtype& vchPubKey = vSolutions[0];
        CKey key;
        if (!key.SetPubKey(vchPubKey))
            return false;
        if (vchBlockSig.empty())
            return false;
        return key.Verify(GetHash(), vchBlockSig);
    }

    return false;
}

bool CheckDiskSpace(uint64_t nAdditionalBytes)
{
    uint64_t nFreeBytesAvailable = filesystem::space(GetDataDir()).available;

    // Check for nMinDiskSpace bytes (currently 50MB)
    if (nFreeBytesAvailable < nMinDiskSpace + nAdditionalBytes)
    {
        fShutdown = true;
        string strMessage = _("Warning: Disk space is low!");
        strMiscWarning = strMessage;
        printf("*** %s\n", strMessage.c_str());
        uiInterface.ThreadSafeMessageBox(strMessage, "Gridcoin", CClientUIInterface::OK | CClientUIInterface::ICON_EXCLAMATION | CClientUIInterface::MODAL);
        StartShutdown();
        return false;
    }
    return true;
}

static filesystem::path BlockFilePath(unsigned int nFile)
{
    string strBlockFn = strprintf("blk%04u.dat", nFile);
    return GetDataDir() / strBlockFn;
}

FILE* OpenBlockFile(unsigned int nFile, unsigned int nBlockPos, const char* pszMode)
{
    if ((nFile < 1) || (nFile == (unsigned int) -1))
        return NULL;
    FILE* file = fopen(BlockFilePath(nFile).string().c_str(), pszMode);
    if (!file)
        return NULL;
    if (nBlockPos != 0 && !strchr(pszMode, 'a') && !strchr(pszMode, 'w'))
    {
        if (fseek(file, nBlockPos, SEEK_SET) != 0)
        {
            fclose(file);
            return NULL;
        }
    }
    return file;
}

static unsigned int nCurrentBlockFile = 1;

FILE* AppendBlockFile(unsigned int& nFileRet)
{
    nFileRet = 0;
    while (true)
    {
        FILE* file = OpenBlockFile(nCurrentBlockFile, 0, "ab");
        if (!file)
            return NULL;
        if (fseek(file, 0, SEEK_END) != 0)
            return NULL;
        // FAT32 file size max 4GB, fseek and ftell max 2GB, so we must stay under 2GB
        if (ftell(file) < (long)(0x7F000000 - MAX_SIZE))
        {
            nFileRet = nCurrentBlockFile;
            return file;
        }
        fclose(file);
        nCurrentBlockFile++;
    }
}

bool LoadBlockIndex(bool fAllowNew)
{
    LOCK(cs_main);

    CBigNum bnTrustedModulus;

    if (fTestNet)
    {
		// GLOBAL TESTNET SETTINGS - R HALFORD
        pchMessageStart[0] = 0xcd;
        pchMessageStart[1] = 0xf2;
        pchMessageStart[2] = 0xc0;
        pchMessageStart[3] = 0xef;
        bnProofOfWorkLimit = bnProofOfWorkLimitTestNet; // 16 bits PoW target limit for testnet
        nStakeMinAge = 1 * 60 * 60; // test net min age is 1 hour
        nCoinbaseMaturity = 10; // test maturity is 10 blocks
		nGrandfather = 196550;
		nNewIndex = 10;
		nNewIndex2 = 36500;
		bRemotePaymentsEnabled = false;
		bOPReturnEnabled = false;
		bOptionPaymentsEnabled = false;
		//1-24-2016
		MAX_OUTBOUND_CONNECTIONS = (int)GetArg("-maxoutboundconnections", 8);
    }


	std::string mode = fTestNet ? "TestNet" : "Prod";
	printf("Mode=%s\r\n",mode.c_str());


    //
    // Load block index
    //
    CTxDB txdb("cr+");
    if (!txdb.LoadBlockIndex())
        return false;

    //
    // Init with genesis block
    //
    if (mapBlockIndex.empty())
    {
        if (!fAllowNew)
            return false;

        // Genesis block - Genesis2
        // MainNet - Official New Genesis Block:
		////////////////////////////////////////
		/*
	 21:58:24 block.nTime = 1413149999
	10/12/14 21:58:24 block.nNonce = 1572771
	10/12/14 21:58:24 block.GetHash = 00000f762f698b5962aa81e38926c3a3f1f03e0b384850caed34cd9164b7f990
	10/12/14 21:58:24 CBlock(hash=00000f762f698b5962aa81e38926c3a3f1f03e0b384850caed34cd9164b7f990, ver=1,
	hashPrevBlock=0000000000000000000000000000000000000000000000000000000000000000,
	hashMerkleRoot=0bd65ac9501e8079a38b5c6f558a99aea0c1bcff478b8b3023d09451948fe841, nTime=1413149999, nBits=1e0fffff, nNonce=1572771, vtx=1, vchBlockSig=)
	10/12/14 21:58:24   Coinbase(hash=0bd65ac950, nTime=1413149999, ver=1, vin.size=1, vout.size=1, nLockTime=0)
    CTxIn(COutPoint(0000000000, 4294967295), coinbase 00012a4531302f31312f313420416e6472656120526f73736920496e647573747269616c20486561742076696e646963617465642077697468204c454e522076616c69646174696f6e)
    CTxOut(empty)
	vMerkleTree: 0bd65ac950

		*/

		const char* pszTimestamp = "10/11/14 Andrea Rossi Industrial Heat vindicated with LENR validation";

        CTransaction txNew;
		//GENESIS TIME
		txNew.nTime = 1413033777;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 0 << CBigNum(42) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].SetEmpty();
        CBlock block;
        block.vtx.push_back(txNew);
        block.hashPrevBlock = 0;
        block.hashMerkleRoot = block.BuildMerkleTree();
        block.nVersion = 1;
		//R&D - Testers Wanted Thread:
		block.nTime    = !fTestNet ? 1413033777 : 1406674534;
		//Official Launch time:
        block.nBits    = bnProofOfWorkLimit.GetCompact();
		block.nNonce = !fTestNet ? 130208 : 22436;
    	printf("starting Genesis Check...");
	    // If genesis block hash does not match, then generate new genesis hash.
        if (block.GetHash() != hashGenesisBlock)
        {
            printf("Searching for genesis block...\n");
            // This will figure out a valid hash and Nonce if you're
            // creating a different genesis block: 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000xFFF
			uint256 hashTarget = CBigNum().SetCompact(block.nBits).getuint256();
            uint256 thash;
            while (true)
            {
            	thash = block.GetHash();
				if (thash <= hashTarget)
                    break;
                if ((block.nNonce & 0xFFF) == 0)
                {
                    printf("nonce %08X: hash = %s (target = %s)\n", block.nNonce, thash.ToString().c_str(), hashTarget.ToString().c_str());
                }
                ++block.nNonce;
                if (block.nNonce == 0)
                {
                    printf("NONCE WRAPPED, incrementing time\n");
                    ++block.nTime;
                }
            }
            printf("block.nTime = %u \n", block.nTime);
            printf("block.nNonce = %u \n", block.nNonce);
            printf("block.GetHash = %s\n", block.GetHash().ToString().c_str());
        }


        block.print();

	    //// debug print

		//GENESIS3: Official Merkle Root
		uint256 merkle_root = uint256("0x5109d5782a26e6a5a5eb76c7867f3e8ddae2bff026632c36afec5dc32ed8ce9f");
		assert(block.hashMerkleRoot == merkle_root);
        assert(block.GetHash() == (!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet));
        assert(block.CheckBlock("LoadBlockIndex",1,10*COIN));

        // Start new block file
        unsigned int nFile;
        unsigned int nBlockPos;
        if (!block.WriteToDisk(nFile, nBlockPos))
            return error("LoadBlockIndex() : writing genesis block to disk failed");
        if (!block.AddToBlockIndex(nFile, nBlockPos, hashGenesisBlock))
            return error("LoadBlockIndex() : genesis block not accepted");

        // ppcoin: initialize synchronized checkpoint
        if (!Checkpoints::WriteSyncCheckpoint((!fTestNet ? hashGenesisBlock : hashGenesisBlockTestNet)))
            return error("LoadBlockIndex() : failed to init sync checkpoint");
    }

    string strPubKey = "";

    // if checkpoint master key changed must reset sync-checkpoint
    if (!txdb.ReadCheckpointPubKey(strPubKey) || strPubKey != CSyncCheckpoint::strMasterPubKey)
    {
        // write checkpoint master key to db
        txdb.TxnBegin();
        if (!txdb.WriteCheckpointPubKey(CSyncCheckpoint::strMasterPubKey))
            return error("LoadBlockIndex() : failed to write new checkpoint master key to db");
        if (!txdb.TxnCommit())
            return error("LoadBlockIndex() : failed to commit new checkpoint master key to db");
        if ((!fTestNet) && !Checkpoints::ResetSyncCheckpoint())
            return error("LoadBlockIndex() : failed to reset sync-checkpoint");
    }

    return true;
}


vector<unsigned char> StringToVector(std::string sData)
{
        vector<unsigned char> v(sData.begin(), sData.end());
		return v;
}

std::string VectorToString(vector<unsigned char> v)
{
        std::string s(v.begin(), v.end());
        return s;
}


std::string ExtractXML(std::string XMLdata, std::string key, std::string key_end)
{

	std::string extraction = "";
	string::size_type loc = XMLdata.find( key, 0 );
	if( loc != string::npos )
	{
		string::size_type loc_end = XMLdata.find( key_end, loc+3);
		if (loc_end != string::npos )
		{
			extraction = XMLdata.substr(loc+(key.length()),loc_end-loc-(key.length()));

		}
	}
	return extraction;
}

std::string ExtractHTML(std::string HTMLdata, std::string tagstartprefix,  std::string tagstart_suffix, std::string tag_end)
{

	std::string extraction = "";
	string::size_type loc = HTMLdata.find( tagstartprefix, 0 );
	if( loc != string::npos )
	{
		//Find the end of the start tag
		string::size_type loc_EOStartTag = HTMLdata.find( tagstart_suffix, loc+tagstartprefix.length());
		if (loc_EOStartTag != string::npos )
		{

			string::size_type loc_end = HTMLdata.find( tag_end, loc_EOStartTag+tagstart_suffix.length());
			if (loc_end != string::npos )
			{
				extraction = HTMLdata.substr(loc_EOStartTag+(tagstart_suffix.length()), loc_end-loc_EOStartTag-(tagstart_suffix.length()));
				extraction = strReplace(extraction,",","");
				if (Contains(extraction,"\r\n"))
				{
					std::vector<std::string> vExtract = split(extraction,"\r\n");
					if (vExtract.size() >= 2)
					{
						extraction = vExtract[2];
						return extraction;
					}
				}
			}
		}
	}
	return extraction;
}


std::string RetrieveMd5(std::string s1)
{
	try
	{
		const char* chIn = s1.c_str();
		unsigned char digest2[16];
		MD5((unsigned char*)chIn, strlen(chIn), (unsigned char*)&digest2);
		char mdString2[33];
		for(int i = 0; i < 16; i++) sprintf(&mdString2[i*2], "%02x", (unsigned int)digest2[i]);
 		std::string xmd5(mdString2);
		return xmd5;
	}
    catch (std::exception &e)
	{
		printf("MD5 INVALID!");
		return "";
	}
}



double Round(double d, int place)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(place) << d ;
	double r = lexical_cast<double>(ss.str());
	return r;
}

double cdbl(std::string s, int place)
{
	if (s=="") s="0";
	s = strReplace(s,"\r","");
	s = strReplace(s,"\n","");
	s = strReplace(s,"a","");
	s = strReplace(s,"a","");
	s = strReplace(s,"b","");
	s = strReplace(s,"c","");
	s = strReplace(s,"d","");
	s = strReplace(s,"e","");
	s = strReplace(s,"f","");
    double r = lexical_cast<double>(s);
	double d = Round(r,place);
	return d;
}


std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in | std::ios::binary);
  if (in)
  {
	  if (fDebug)  printf("loading file to string %s","test");

    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

std::ifstream::pos_type filesize2(const char* filename)
{
    std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
    in.seekg(0, std::ifstream::end);
    return in.tellg();
}


std::string deletefile(std::string filename)
{
	std::string buffer;
	std::string line;
	ifstream myfile;
    if (fDebug10) printf("loading file to string %s",filename.c_str());
	filesystem::path path = filename;
	if (!filesystem::exists(path)) {
		printf("the file does not exist %s",path.string().c_str());
		return "-1";
	}
	int deleted = remove(filename.c_str());
	if (deleted != 0) return "Error deleting.";
	return "";
}


int GetFilesize(FILE* file)
{
    int nSavePos = ftell(file);
    int nFilesize = -1;
    if (fseek(file, 0, SEEK_END) == 0)
        nFilesize = ftell(file);
    fseek(file, nSavePos, SEEK_SET);
    return nFilesize;
}




bool WriteKey(std::string sKey, std::string sValue)
{
	// Allows Gridcoin to store the key value in the config file.
	boost::filesystem::path pathConfigFile(GetArg("-conf", "gridcoinresearch.conf"));
    if (!pathConfigFile.is_complete()) pathConfigFile = GetDataDir(false) / pathConfigFile;
	if (!filesystem::exists(pathConfigFile))  return false; 
	boost::to_lower(sKey);
	std::string sLine = "";
	ifstream streamConfigFile;
	streamConfigFile.open(pathConfigFile.string().c_str());
	std::string sConfig = "";
	bool fWritten = false;
    if(streamConfigFile)
	{
	   while(getline(streamConfigFile, sLine))
	   {
		   	std::vector<std::string> vEntry = split(sLine,"=");
			if (vEntry.size() == 2)
			{
				std::string sSourceKey = vEntry[0];
				std::string sSourceValue = vEntry[1];
				boost::to_lower(sSourceKey);

				if (sSourceKey==sKey) 
				{
					sSourceValue = sValue;
					sLine = sSourceKey + "=" + sSourceValue;
					fWritten=true;
				}
			}
			sLine =	strReplace(sLine,"\r","");
			sLine =	strReplace(sLine,"\n","");
			sLine += "\r\n";
			sConfig += sLine;
	   }
	}
	if (!fWritten) 
	{
		sLine = sKey + "=" + sValue + "\r\n";
		sConfig += sLine;
	}
	
	streamConfigFile.close();

	FILE *outFile = fopen(pathConfigFile.string().c_str(),"w");
	fputs(sConfig.c_str(), outFile);
	fclose(outFile);

	ReadConfigFile(mapArgs, mapMultiArgs);
	return true;
}




std::string getfilecontents(std::string filename)
{
	std::string buffer;
	std::string line;
	ifstream myfile;
    if (fDebug10) printf("loading file to string %s",filename.c_str());

	filesystem::path path = filename;

	if (!filesystem::exists(path)) {
		printf("the file does not exist %s",path.string().c_str());
		return "-1";
	}

	 FILE *file = fopen(filename.c_str(), "rb");
     CAutoFile filein = CAutoFile(file, SER_DISK, CLIENT_VERSION);
	 int fileSize = GetFilesize(filein);
     filein.fclose();

	 myfile.open(filename.c_str());

    buffer.reserve(fileSize);
    if (fDebug10) printf("opening file %s",filename.c_str());

	if(myfile)
	{
	  while(getline(myfile, line))
	  {
			buffer = buffer + line + "\r\n";
	  }
	}
	myfile.close();
	return buffer;
}


bool IsCPIDValidv3(std::string cpidv2, bool allow_investor)
{
	// Used for checking the local cpid
	bool result=false;
	if (allow_investor) if (cpidv2 == "INVESTOR" || cpidv2=="investor") return true;
	if (cpidv2.length() < 34) return false;
	result = CPID_IsCPIDValid(cpidv2.substr(0,32),cpidv2,0);
	return result;
}

bool IsCPIDValidv2(MiningCPID& mc, int height)
{
	//09-25-2016: Transition to CPID Keypairs.
	if (height < nGrandfather) return true;
	bool result = false;
	int cpidV2CutOverHeight = fTestNet ? 0 : 97000;
	int cpidV3CutOverHeight = fTestNet ? 196300 : 725000;
	if (height < cpidV2CutOverHeight)
	{
		result = IsCPIDValid_Retired(mc.cpid,mc.enccpid);
	}
	else if (height >= cpidV2CutOverHeight && height <= cpidV3CutOverHeight)
	{
	    if (mc.cpid == "INVESTOR" || mc.cpid=="investor") return true;
        result = CPID_IsCPIDValid(mc.cpid, mc.cpidv2, (uint256)mc.lastblockhash);
	}
	else if (height >= cpidV3CutOverHeight)
	{
	    if (mc.cpid == "INVESTOR" || mc.cpid=="investor") return true;
		// V3 requires a beacon, a beacon public key and a valid block signature signed by the CPID's private key
		result = VerifyCPIDSignature(mc.cpid,mc.lastblockhash,mc.BoincSignature);
	}

	return result;
}


bool IsLocalCPIDValid(StructCPID& structcpid)
{

	bool new_result = IsCPIDValidv3(structcpid.cpidv2,true);
	return new_result;

}



bool IsCPIDValid_Retired(std::string cpid, std::string ENCboincpubkey)
{

	try
	{
			if(cpid=="" || cpid.length() < 5)
			{
				printf("CPID length empty.");
				return false;
			}
			if (cpid=="INVESTOR") return true;
			if (ENCboincpubkey == "" || ENCboincpubkey.length() < 5)
			{
					if (fDebug10) printf("ENCBpk length empty.");
					return false;
			}
			std::string bpk = AdvancedDecrypt(ENCboincpubkey);
			std::string bpmd5 = RetrieveMd5(bpk);
			if (bpmd5==cpid) return true;
			if (fDebug10) printf("Md5<>cpid, md5 %s cpid %s  root bpk %s \r\n     ",bpmd5.c_str(), cpid.c_str(),bpk.c_str());

			return false;
	}
	catch (std::exception &e)
	{
				printf("Error while resolving CPID\r\n");
				return false;
	}
	catch(...)
	{
				printf("Error while Resolving CPID[2].\r\n");
				return false;
	}
	return false;

}

double Cap(double dAmt, double Ceiling)
{
	if (dAmt > Ceiling) dAmt = Ceiling;
	return dAmt;
}

double Lowest(double dAmt1, double dAmt2)
{
	if (dAmt1 < dAmt2)
	{
		return dAmt1;
	}
	else
	{
		return dAmt2;
	}
}

int64_t Floor(int64_t iAmt1, int64_t iAmt2)
{
	int64_t iOut = 0;
	if (iAmt1 <= iAmt2)
	{
		iOut = iAmt1;
	}
	else
	{
		iOut = iAmt2;
	}
	return iOut;

}

double coalesce(double mag1, double mag2)
{
	if (mag1 > 0) return mag1;
	return mag2;
}

double GetTotalOwedAmount(std::string cpid)
{
	StructCPID o = GetInitializedStructCPID2(cpid,mvMagnitudes);
	return o.totalowed;
}

double GetOwedAmount(std::string cpid)
{
	if (mvMagnitudes.size() > 1)
	{
		StructCPID m = GetInitializedStructCPID2(cpid,mvMagnitudes);
		if (m.initialized) return m.owed;
		return 0;
	}
	return 0;
}


double GetOutstandingAmountOwed(StructCPID &mag, std::string cpid, int64_t locktime,
	double& total_owed, double block_magnitude)
{
	// Gridcoin Payment Magnitude Unit in RSA Owed calculation ensures rewards are capped at MaxBlockSubsidy*BLOCKS_PER_DAY
	// Payment date range is stored in HighLockTime-LowLockTime
	// If newbie has not participated for 14 days, use earliest payment in chain to assess payment window
	// (Important to prevent e-mail change attacks) - Calculate payment timespan window in days
	try
	{
		double payment_timespan = (GetAdjustedTime() - mag.EarliestPaymentTime)/38400;
		if (payment_timespan < 2) payment_timespan =  2;
		if (payment_timespan > 10) payment_timespan = 14;
		mag.PaymentTimespan = Round(payment_timespan,0);
		double research_magnitude = 0;
		// Get neural network magnitude:
		StructCPID stDPOR = GetInitializedStructCPID2(cpid,mvDPOR);
		research_magnitude = LederstrumpfMagnitude2(stDPOR.Magnitude,locktime);
		double owed_standard = payment_timespan * Cap(research_magnitude*GetMagnitudeMultiplier(locktime),
			GetMaximumBoincSubsidy(locktime)*5);
		double owed_network_cap = payment_timespan * GRCMagnitudeUnit(locktime) * research_magnitude;
		double owed = Lowest(owed_standard,owed_network_cap);
		double paid = mag.payments;
		double outstanding = Lowest(owed-paid, GetMaximumBoincSubsidy(locktime)*5);
		total_owed = owed;
		//if (outstanding < 0) outstanding=0;
		return outstanding;
	}
	catch (std::exception &e)
	{
			printf("Error while Getting outstanding amount owed.");
			return 0;
	}
    catch(...)
	{
			printf("Error while Getting outstanding amount owed.");
			return 0;
	}
}

bool BlockNeedsChecked(int64_t BlockTime)
{
	if (IsLockTimeWithin14days((double)BlockTime))
	{
		if (fColdBoot) return false;
		bool fOut = OutOfSyncByMoreThan(30);
		return !fOut;
	}
	else
	{
		return false;
	}
}

bool IsLockTimeWithin14days(double locktime)
{
	//Within 14 days
	double nCutoff =  GetAdjustedTime() - (60*60*24*14);
	if (locktime < nCutoff) return false;
	return true;
}

bool LockTimeRecent(double locktime)
{
	//Returns true if adjusted time is within 45 minutes
	double nCutoff =  GetAdjustedTime() - (60*45);
	if (locktime < nCutoff) return false;
	return true;
}

bool IsLockTimeWithinMinutes(double locktime, int minutes)
{
	double nCutoff = GetAdjustedTime() - (60*minutes);
	if (locktime < nCutoff) return false;
	return true;
}

bool IsLockTimeWithinMinutes(int64_t locktime, int minutes)
{
	double nCutoff = GetAdjustedTime() - (60*minutes);
	if (locktime < nCutoff) return false;
	return true;
}


double GetMagnitudeWeight(double LockTime)
{
	double age = ( GetAdjustedTime() - LockTime)/86400;
	double inverse = 14-age;
	if (inverse < 1) inverse=1;
	return inverse*inverse;
}




void RemoveNetworkMagnitude(double LockTime, std::string cpid, MiningCPID bb, double mint, bool IsStake)
{
        if (!IsLockTimeWithin14days(LockTime)) return;
		StructCPID structMagnitude = GetInitializedStructCPID2(cpid,mvMagnitudes);
		structMagnitude.projectname = bb.projectname;
		structMagnitude.entries--;
		if (IsStake)
		{
			double interest = (double)mint - (double)bb.ResearchSubsidy;
			structMagnitude.payments -= bb.ResearchSubsidy;
			structMagnitude.interestPayments = structMagnitude.interestPayments - interest;
		    structMagnitude.LastPaymentTime = 0;
		}
		structMagnitude.cpid = cpid;
		double total_owed = 0;
		mvMagnitudes[cpid] = structMagnitude;
		structMagnitude.owed = GetOutstandingAmountOwed(structMagnitude,cpid,LockTime,total_owed,bb.Magnitude);
		structMagnitude.totalowed = total_owed;
		mvMagnitudes[cpid] = structMagnitude;
}






void AdjustTimestamps(StructCPID& strCPID, double timestamp, double subsidy)
{
		if (timestamp > strCPID.LastPaymentTime && subsidy > 0) strCPID.LastPaymentTime = timestamp;
		if (timestamp < strCPID.EarliestPaymentTime) strCPID.EarliestPaymentTime = timestamp;
}




void AddResearchMagnitude(CBlockIndex* pIndex)
{
	    // Headless critical section
		if (pIndex->nResearchSubsidy > 0)
		{
			try
			{
				StructCPID stMag = GetInitializedStructCPID2(pIndex->sCPID,mvMagnitudesCopy);
				stMag.cpid = pIndex->sCPID;
				stMag.GRCAddress = pIndex->sGRCAddress;
				if ((double)pIndex->nHeight > stMag.LastBlock)
				{
					stMag.LastBlock = (double)pIndex->nHeight;
				}
				stMag.entries++;
				stMag.payments += pIndex->nResearchSubsidy;
				stMag.interestPayments += pIndex->nInterestSubsidy;

				AdjustTimestamps(stMag,(double)pIndex->nTime,pIndex->nResearchSubsidy);
				// Track detailed payments made to each CPID
				stMag.PaymentTimestamps         += RoundToString((double)pIndex->nTime,0) + ",";
				stMag.PaymentAmountsResearch    += RoundToString(pIndex->nResearchSubsidy,2) + ",";
				stMag.PaymentAmountsInterest    += RoundToString(pIndex->nInterestSubsidy,2) + ",";
				stMag.PaymentAmountsBlocks      += RoundToString((double)pIndex->nHeight,0) + ",";
     			stMag.Accuracy++;
				stMag.AverageRAC = stMag.rac / (stMag.entries+.01);
				double total_owed = 0;
				stMag.owed = GetOutstandingAmountOwed(stMag,
					pIndex->sCPID,(double)pIndex->nTime,total_owed,pIndex->nMagnitude);

				stMag.totalowed = total_owed;
				mvMagnitudesCopy[pIndex->sCPID] = stMag;
			}
			catch (bad_alloc ba)
			{
				printf("\r\nBad Allocation in AddResearchMagnitude() \r\n");
			}
			catch(...)
			{
				printf("Exception in AddResearchMagnitude() \r\n");
			}
		}
}




void AddNMRetired(double height, double LockTime, std::string cpid, MiningCPID bb)
{
	try
	{
	    StructCPID stMag = GetInitializedStructCPID2(cpid,mvMagnitudes);
		stMag.cpid = cpid;
		stMag.GRCAddress = bb.GRCAddress;
		if (height > stMag.LastBlock)
		{
			stMag.LastBlock = height;
		}
		stMag.projectname = bb.projectname;
		stMag.rac += bb.rac;
		stMag.entries++;
		stMag.payments += bb.ResearchSubsidy;
		stMag.interestPayments += bb.InterestSubsidy;
		AdjustTimestamps(stMag,LockTime,bb.ResearchSubsidy);
		// Per RTM 6-27-2015 - Track detailed payments made to each CPID
		stMag.PaymentTimestamps         += RoundToString(LockTime,0) + ",";
		stMag.PaymentAmountsResearch    += RoundToString(bb.ResearchSubsidy,2) + ",";
		stMag.PaymentAmountsInterest    += RoundToString(bb.InterestSubsidy,2) + ",";
		stMag.PaymentAmountsBlocks      += RoundToString((double)height,0) + ",";
     	stMag.Accuracy++;
	    stMag.AverageRAC = stMag.rac / (stMag.entries+.01);
	    double total_owed = 0;
	    stMag.owed = GetOutstandingAmountOwed(stMag,cpid,LockTime,total_owed,bb.Magnitude);
	    stMag.totalowed = total_owed;
	    mvMagnitudes[cpid] = stMag;
	}
	catch (std::exception &e)
	{
			printf("Error while Adding Network Magnitude.");
	}
    catch(...)
	{
			printf("Error while Adding Network Magnitude.");
	}
}




bool GetEarliestStakeTime(std::string grcaddress, std::string cpid)
{
    if (nBestHeight < 15)
	{
		mvApplicationCacheTimestamp["nGRCTime"] = GetAdjustedTime();
		mvApplicationCacheTimestamp["nCPIDTime"] = GetAdjustedTime();
		return true;
	}
	if (IsLockTimeWithinMinutes(nLastGRCtallied,100)) return true;
	nLastGRCtallied = GetAdjustedTime();
	int64_t nGRCTime = 0;
	int64_t nCPIDTime = 0;
	CBlock block;
	int64_t nStart = GetTimeMillis();
	LOCK(cs_main);
	{
		    int nMaxDepth = nBestHeight;
			int nLookback = BLOCKS_PER_DAY*6*30;  //6 months back for performance
			int nMinDepth = nMaxDepth - nLookback;
			if (nMinDepth < 2) nMinDepth = 2;
			// Start at the earliest block index:
			CBlockIndex* pblockindex = FindBlockByHeight(nMinDepth);
		    while (pblockindex->nHeight < nMaxDepth-1)
			{
						pblockindex = pblockindex->pnext;
						if (pblockindex == pindexBest) break;
						if (pblockindex == NULL || !pblockindex->IsInMainChain()) continue;
						std::string myCPID = "";
						if (pblockindex->nHeight < nNewIndex)
						{
							//Between block 1 and nNewIndex, unfortunately, we have to read from disk.
							block.ReadFromDisk(pblockindex);
							std::string hashboinc = "";
							if (block.vtx.size() > 0) hashboinc = block.vtx[0].hashBoinc;
							MiningCPID bb = DeserializeBoincBlock(hashboinc);
							myCPID = bb.cpid;
						}
						else
						{
							myCPID = pblockindex->sCPID;
						}
						if (cpid == myCPID && nCPIDTime==0)
						{
							nCPIDTime = pblockindex->nTime;
							nGRCTime = pblockindex->nTime;
							break;
						}
			}
	}
	int64_t EarliestStakedWalletTx = GetEarliestWalletTransaction();
	if (EarliestStakedWalletTx > 0 && EarliestStakedWalletTx < nGRCTime) nGRCTime = EarliestStakedWalletTx;
	if (fTestNet) nGRCTime -= (86400*30);
	if (nGRCTime <= 0)  nGRCTime = GetAdjustedTime();
	if (nCPIDTime <= 0) nCPIDTime = GetAdjustedTime();

	printf("Loaded staketime from index in %f", (double)(GetTimeMillis() - nStart));
	printf("CPIDTime %f, GRCTime %f, WalletTime %f \r\n",(double)nCPIDTime,(double)nGRCTime,(double)EarliestStakedWalletTx);
	mvApplicationCacheTimestamp["nGRCTime"] = nGRCTime;
	mvApplicationCacheTimestamp["nCPIDTime"] = nCPIDTime;
	return true;
}

HashSet GetCPIDBlockHashes(const std::string& cpid)
{
    auto hashes = mvCPIDBlockHashes.find(cpid);
    return hashes != mvCPIDBlockHashes.end()
        ? hashes->second
        : HashSet();
}

void AddCPIDBlockHash(const std::string& cpid, const uint256& blockhash)
{
    // Add block hash to CPID hash set.
    mvCPIDBlockHashes[cpid].insert(blockhash);
}

StructCPID GetLifetimeCPID(std::string cpid, std::string sCalledFrom)
{
    //Eliminates issues with reorgs, disconnects, double counting, etc.. 
    if (cpid.empty() || cpid=="INVESTOR")
        return GetInitializedStructCPID2("INVESTOR",mvResearchAge);
	
    if (fDebug10) printf(" {GLC %s} ",sCalledFrom.c_str());

    const HashSet& hashes = GetCPIDBlockHashes(cpid);
    ZeroOutResearcherTotals(cpid);

    for (HashSet::iterator it = hashes.begin(); it != hashes.end(); ++it)
    {
        const uint256& uHash = *it;

        // Ensure that we have this block.
        if (mapBlockIndex.count(uHash) == 0)
           continue;
        
        // Ensure that the block is valid.    
        CBlockIndex* pblockindex = mapBlockIndex[uHash];
        if(pblockindex == NULL ||
           pblockindex->IsInMainChain() == false ||
           pblockindex->sCPID != cpid)
            continue;

        // Block located and verified.
        StructCPID stCPID = GetInitializedStructCPID2(pblockindex->sCPID,mvResearchAge);
        if (pblockindex->nHeight > stCPID.LastBlock && pblockindex->nResearchSubsidy > 0)
        {
            stCPID.LastBlock = pblockindex->nHeight;
            stCPID.BlockHash = pblockindex->GetBlockHash().GetHex();
        }
        stCPID.InterestSubsidy += pblockindex->nInterestSubsidy;
        stCPID.ResearchSubsidy += pblockindex->nResearchSubsidy;
        stCPID.Accuracy++;
        if (pblockindex->nMagnitude > 0)
        {
            stCPID.TotalMagnitude += pblockindex->nMagnitude;
            stCPID.ResearchAverageMagnitude = stCPID.TotalMagnitude/(stCPID.Accuracy+.01);
        }

        if (pblockindex->nTime < stCPID.LowLockTime)  stCPID.LowLockTime  = pblockindex->nTime;
        if (pblockindex->nTime > stCPID.HighLockTime) stCPID.HighLockTime = pblockindex->nTime;
        mvResearchAge[pblockindex->sCPID]=stCPID;
    }

    return GetInitializedStructCPID2(cpid, mvResearchAge);
}

MiningCPID GetInitializedMiningCPID(std::string name,std::map<std::string, MiningCPID>& vRef)
{
   MiningCPID& cpid = vRef[name];
	if (!cpid.initialized)
	{
			    cpid = GetMiningCPID();
				cpid.initialized=true;
				cpid.LastPaymentTime = 0;
	}

   return cpid;
}


StructCPID GetInitializedStructCPID2(std::string name, std::map<std::string, StructCPID>& vRef)
{
	try
	{
      StructCPID& cpid = vRef[name];
		if (!cpid.initialized)
		{
				cpid = GetStructCPID();
				cpid.initialized=true;
				cpid.LowLockTime = 99999999999;
				cpid.HighLockTime = 0;
				cpid.LastPaymentTime = 0;
				cpid.EarliestPaymentTime = 99999999999;
				cpid.Accuracy = 0;
				return cpid;
		}
		else
		{
				return cpid;
		}
	}
	catch (bad_alloc ba)
	{
		printf("Bad alloc caught in GetInitializedStructCpid2 for %s",name.c_str());
      return GetStructCPID();
	}
	catch(...)
	{
		printf("Exception caught in GetInitializedStructCpid2 for %s",name.c_str());
      return GetStructCPID();
	}
}




bool RetiredTN(bool Forcefully)
{
	//Iterate throught last 14 days, tally network averages
    if (nBestHeight < 15)
	{
		bNetAveragesLoaded = true;
		return true;
	}

	if (Forcefully) nLastTallied = 0;
	int timespan = fTestNet ? 1 : 5;
	if (IsLockTimeWithinMinutes(nLastTallied,timespan))
	{
		bNetAveragesLoaded=true;
		return true;
	}

	printf("Gathering network avgs (begin)");
	nLastTallied = GetAdjustedTime();
	bNetAveragesLoaded = false;
	bool superblockloaded = false;
	double NetworkPayments = 0;
	double NetworkInterest = 0;
					//7-5-2015 - R Halford - Start block and End block must be an exact range agreed by the network:
					int nMaxDepth = (nBestHeight-CONSENSUS_LOOKBACK) - ( (nBestHeight-CONSENSUS_LOOKBACK) % BLOCK_GRANULARITY);
					int nLookback = BLOCKS_PER_DAY*14; //Daily block count * Lookback in days = 14 days
					int nMinDepth = (nMaxDepth - nLookback) - ( (nMaxDepth-nLookback) % BLOCK_GRANULARITY);
					if (fDebug3) printf("START BLOCK %f, END BLOCK %f ",(double)nMaxDepth,(double)nMinDepth);
					if (nMinDepth < 2)              nMinDepth = 2;
					if (mvMagnitudes.size() > 0) 	mvMagnitudes.clear();
					CBlock block;
					CBlockIndex* pblockindex = pindexBest;

					while (pblockindex->nHeight > nMaxDepth)
					{
						if (!pblockindex || !pblockindex->pprev) return false;
						pblockindex = pblockindex->pprev;
						if (pblockindex == pindexGenesisBlock) return false;
					}

					if (fDebug) printf("\r\n GetMoneySupply::Max block %f",(double)pblockindex->nHeight);

		    		while (pblockindex->nHeight > nMinDepth)
					{
						if (!pblockindex || !pblockindex->pprev) return false;   //Avoid segfault
						pblockindex = pblockindex->pprev;
						if (pblockindex == pindexGenesisBlock) return false;
						if (pblockindex == NULL || !pblockindex->IsInMainChain()) continue;
						MiningCPID bb;
						
						if (!block.ReadFromDisk(pblockindex)) continue;
						if (block.vtx.size() > 0)
						{
								if (block.vtx[0].hashBoinc.empty()) continue;
								bb = DeserializeBoincBlock(block.vtx[0].hashBoinc);
						}
						else continue;

						NetworkPayments += bb.ResearchSubsidy;
						NetworkInterest += bb.InterestSubsidy;
						// Insert CPID, Magnitude, Payments
						AddNMRetired((double)pblockindex->nHeight,pblockindex->nTime,bb.cpid,bb);
						if (!superblockloaded && bb.superblock.length() > 20)
						{
								    std::string superblock = UnpackBinarySuperblock(bb.superblock);
									if (VerifySuperblock(superblock,pblockindex->nHeight))
									{
	    									LoadSuperblock(superblock,pblockindex->nTime,pblockindex->nHeight);
											superblockloaded=true;
											if (fDebug3) printf(" Superblock Loaded %f \r\n",(double)pblockindex->nHeight);
									}
							}

					}
					if (pblockindex && fDebug10)	printf("Min block %f \r\n",(double)pblockindex->nHeight);
					StructCPID network = GetInitializedStructCPID2("NETWORK",mvNetwork);
					network.projectname="NETWORK";
					network.payments = NetworkPayments;
					network.InterestSubsidy = NetworkInterest;
					mvNetwork["NETWORK"] = network;
					TallyMagnitudesInSuperblock();
					GetNextProject(false);
					if (fDebug3) printf(".Done.\r\n %f",(double)0);
					bTallyStarted = false;
					bNetAveragesLoaded = true;
					return true;

	bNetAveragesLoaded=true;
	return false;
}



bool ComputeNeuralNetworkSupermajorityHashes()
{
    if (nBestHeight < 15)  return true;
	if (IsLockTimeWithinMinutes(nLastTalliedNeural,5))
	{
		return true;
	}
	nLastTalliedNeural = GetAdjustedTime();
	//Clear the neural network hash buffer
	if (mvNeuralNetworkHash.size() > 0)  mvNeuralNetworkHash.clear();
	if (mvNeuralVersion.size() > 0)  mvNeuralVersion.clear();
	if (mvCurrentNeuralNetworkHash.size() > 0) mvCurrentNeuralNetworkHash.clear();

	//Clear the votes
	WriteCache("neuralsecurity","pending","0",GetAdjustedTime());
	ClearCache("neuralsecurity");
	try
	{
		int nMaxDepth = nBestHeight;
		int nLookback = 100;
		int nMinDepth = (nMaxDepth - nLookback);
		if (nMinDepth < 2)   nMinDepth = 2;
		CBlock block;
		CBlockIndex* pblockindex = pindexBest;
		while (pblockindex->nHeight > nMinDepth)
		{
			if (!pblockindex || !pblockindex->pprev) return false;
			pblockindex = pblockindex->pprev;
            if (pblockindex == pindexGenesisBlock) return false;
			if (!pblockindex->IsInMainChain()) continue;
			block.ReadFromDisk(pblockindex);
			std::string hashboinc = "";
			if (block.vtx.size() > 0) hashboinc = block.vtx[0].hashBoinc;
			if (!hashboinc.empty())
			{
				MiningCPID bb = DeserializeBoincBlock(hashboinc);
				//If block is pending: 7-25-2015
				if (bb.superblock.length() > 20)
				{
					std::string superblock = UnpackBinarySuperblock(bb.superblock);
					if (VerifySuperblock(superblock,pblockindex->nHeight))
					{
						WriteCache("neuralsecurity","pending",RoundToString((double)pblockindex->nHeight,0),GetAdjustedTime());
					}
				}
				if (!bb.clientversion.empty())
				{
					IncrementVersionCount(bb.clientversion);
				}
				//Increment Neural Network Hashes Supermajority (over the last N blocks)
				IncrementNeuralNetworkSupermajority(bb.NeuralHash,bb.GRCAddress,(nMaxDepth-pblockindex->nHeight)+10);
				IncrementCurrentNeuralNetworkSupermajority(bb.CurrentNeuralHash,bb.GRCAddress,(nMaxDepth-pblockindex->nHeight)+10);

			}
		}

		if (fDebug3) printf(".11.");
	}
	catch (std::exception &e)
	{
			printf("Neural Error while memorizing hashes.\r\n");
	}
    catch(...)
	{
		printf("Neural error While Memorizing Hashes! [1]\r\n");
	}
	return true;

}

MiningCPID GetBoincBlockByHeight(int ii, double& mint, int64_t& nTime)
{
	CBlock block;
	MiningCPID bb;
	CBlockIndex* pblockindex = FindBlockByHeight(ii);
	bb.initialized=false;
	if (pblockindex == NULL) return bb;
	if (pblockindex->pnext == NULL) return bb;
	if (!pblockindex || !pblockindex->IsInMainChain()) return bb;


	if (block.ReadFromDisk(pblockindex))
	{
		nTime = block.nTime;
		std::string hashboinc = "";
		mint = CoinToDouble(pblockindex->nMint);
		if (block.vtx.size() > 0) hashboinc = block.vtx[0].hashBoinc;
		bb = DeserializeBoincBlock(hashboinc);
		bb.initialized=true;
		return bb;
	}
	return bb;
}



void AddProjectRAC(MiningCPID bb,double& NetworkRAC, double& NetworkMagnitude)
{
	//RETIRE
	StructCPID network = GetInitializedStructCPID2(bb.projectname,mvNetwork);
	network.projectname = bb.projectname;
	network.rac += bb.rac;
	NetworkRAC += bb.rac;
	network.TotalRAC = NetworkRAC;
	NetworkMagnitude += bb.Magnitude;
	network.entries++;
	mvNetwork[bb.projectname] = network;
}


bool TallyResearchAverages(bool Forcefully)
{
	//Iterate throught last 14 days, tally network averages
    if (nBestHeight < 15)
	{
		bNetAveragesLoaded = true;
		return true;
	}

	//if (Forcefully) nLastTallied = 0;
	int timespan = fTestNet ? 2 : 6;
	if (IsLockTimeWithinMinutes(nLastTallied,timespan))
	{
		bNetAveragesLoaded=true;
		return true;
	}

	//8-27-2016
	 int64_t nStart = GetTimeMillis();


	if (fDebug3) printf("Tallying Research Averages (begin) ");
	nLastTallied = GetAdjustedTime();
	bNetAveragesLoaded = false;
	bool superblockloaded = false;
	double NetworkPayments = 0;
	double NetworkInterest = 0;
	
 						//Consensus Start/End block:
						int nMaxDepth = (nBestHeight-CONSENSUS_LOOKBACK) - ( (nBestHeight-CONSENSUS_LOOKBACK) % BLOCK_GRANULARITY);
						int nLookback = BLOCKS_PER_DAY * 14; //Daily block count * Lookback in days
						int nMinDepth = (nMaxDepth - nLookback) - ( (nMaxDepth-nLookback) % BLOCK_GRANULARITY);
						if (fDebug3) printf("START BLOCK %f, END BLOCK %f ",(double)nMaxDepth,(double)nMinDepth);
						if (nMinDepth < 2)              nMinDepth = 2;
						mvMagnitudesCopy.clear();
						int iRow = 0;
						//CBlock block;
						CBlockIndex* pblockindex = pindexBest;
						if (!pblockindex)
						{
								bTallyStarted = false;
								bNetAveragesLoaded = true;
								return true;
						}
						while (pblockindex->nHeight > nMaxDepth)
						{
							if (!pblockindex || !pblockindex->pprev || pblockindex == pindexGenesisBlock) return false;
							pblockindex = pblockindex->pprev;
						}

						if (fDebug3) printf("Max block %f, seektime %f",(double)pblockindex->nHeight,(double)GetTimeMillis()-nStart);
						nStart=GetTimeMillis();

   
						// Headless critical section ()
		try
		{
						while (pblockindex->nHeight > nMinDepth)
						{
							if (!pblockindex || !pblockindex->pprev) return false;
							pblockindex = pblockindex->pprev;
							if (pblockindex == pindexGenesisBlock) return false;
							if (!pblockindex->IsInMainChain()) continue;
							NetworkPayments += pblockindex->nResearchSubsidy;
							NetworkInterest += pblockindex->nInterestSubsidy;
							AddResearchMagnitude(pblockindex);

							iRow++;
							if (IsSuperBlock(pblockindex) && !superblockloaded)
							{
								MiningCPID bb = GetBoincBlockByIndex(pblockindex);
								if (bb.superblock.length() > 20)
								{
										std::string superblock = UnpackBinarySuperblock(bb.superblock);
										if (VerifySuperblock(superblock,pblockindex->nHeight))
										{
												LoadSuperblock(superblock,pblockindex->nTime,pblockindex->nHeight);
												superblockloaded=true;
												if (fDebug3) printf(" Superblock Loaded %f \r\n",(double)pblockindex->nHeight);
										}
								}
							}

						}
						// End of critical section
						if (fDebug3) printf("TNA loaded in %f",(double)GetTimeMillis()-nStart);
						nStart=GetTimeMillis();


						if (pblockindex)
						{
							if (fDebug3) printf("Min block %f, Rows %f \r\n",(double)pblockindex->nHeight,(double)iRow);
							StructCPID network = GetInitializedStructCPID2("NETWORK",mvNetworkCopy);
							network.projectname="NETWORK";
							network.payments = NetworkPayments;
							network.InterestSubsidy = NetworkInterest;
							mvNetworkCopy["NETWORK"] = network;
							if(fDebug3) printf(" TMIS1 ");
							TallyMagnitudesInSuperblock();
						}
						// 11-19-2015 Copy dictionaries to live RAM
						mvDPOR = mvDPORCopy;
						mvMagnitudes = mvMagnitudesCopy;
						mvNetwork = mvNetworkCopy;
						bTallyStarted = false;
						bNetAveragesLoaded = true;
						return true;
		}
		catch (bad_alloc ba)
		{
			printf("Bad Alloc while tallying network averages. [1]\r\n");
			bNetAveragesLoaded=true;
            nLastTallied = 0;
		}
		catch(...)
		{
			printf("Error while tallying network averages. [1]\r\n");
			bNetAveragesLoaded=true;
            nLastTallied = 0;
		}

		if (fDebug3) printf("NA loaded in %f",(double)GetTimeMillis()-nStart);
						
	    bNetAveragesLoaded=true;
	    return false;
}



bool TallyNetworkAverages(bool Forcefully)
{
	if (IsResearchAgeEnabled(pindexBest->nHeight))
	{
		return TallyResearchAverages(Forcefully);
	}
	else
	{
		return RetiredTN(Forcefully);
	}
}


void PrintBlockTree()
{
    AssertLockHeld(cs_main);
    // pre-compute tree structure
    map<CBlockIndex*, vector<CBlockIndex*> > mapNext;
    for (map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.begin(); mi != mapBlockIndex.end(); ++mi)
    {
        CBlockIndex* pindex = (*mi).second;
        mapNext[pindex->pprev].push_back(pindex);
    }

    vector<pair<int, CBlockIndex*> > vStack;
    vStack.push_back(make_pair(0, pindexGenesisBlock));

    int nPrevCol = 0;
    while (!vStack.empty())
    {
        int nCol = vStack.back().first;
        CBlockIndex* pindex = vStack.back().second;
        vStack.pop_back();

        // print split or gap
        if (nCol > nPrevCol)
        {
            for (int i = 0; i < nCol-1; i++)
                printf("| ");
            printf("|\\\n");
        }
        else if (nCol < nPrevCol)
        {
            for (int i = 0; i < nCol; i++)
                printf("| ");
            printf("|\n");
       }
        nPrevCol = nCol;

        // print columns
        for (int i = 0; i < nCol; i++)
            printf("| ");

        // print item
        CBlock block;
        block.ReadFromDisk(pindex);
        printf("%d (%u,%u) %s  %08x  %s  mint %7s  tx %" PRIszu "",
            pindex->nHeight,
            pindex->nFile,
            pindex->nBlockPos,
            block.GetHash().ToString().c_str(),
            block.nBits,
            DateTimeStrFormat("%x %H:%M:%S", block.GetBlockTime()).c_str(),
            FormatMoney(pindex->nMint).c_str(),
            block.vtx.size());

        PrintWallets(block);

        // put the main time-chain first
        vector<CBlockIndex*>& vNext = mapNext[pindex];
        for (unsigned int i = 0; i < vNext.size(); i++)
        {
            if (vNext[i]->pnext)
            {
                swap(vNext[0], vNext[i]);
                break;
            }
        }

        // iterate children
        for (unsigned int i = 0; i < vNext.size(); i++)
            vStack.push_back(make_pair(nCol+i, vNext[i]));
    }
}

bool LoadExternalBlockFile(FILE* fileIn)
{
    int64_t nStart = GetTimeMillis();

    int nLoaded = 0;
    {
        LOCK(cs_main);
        try {
            CAutoFile blkdat(fileIn, SER_DISK, CLIENT_VERSION);
            unsigned int nPos = 0;
            while (nPos != (unsigned int)-1 && blkdat.good() && !fRequestShutdown)
            {
                unsigned char pchData[65536];
                do {
                    fseek(blkdat, nPos, SEEK_SET);
                    int nRead = fread(pchData, 1, sizeof(pchData), blkdat);
                    if (nRead <= 8)
                    {
                        nPos = (unsigned int)-1;
                        break;
                    }
                    void* nFind = memchr(pchData, pchMessageStart[0], nRead+1-sizeof(pchMessageStart));
                    if (nFind)
                    {
                        if (memcmp(nFind, pchMessageStart, sizeof(pchMessageStart))==0)
                        {
                            nPos += ((unsigned char*)nFind - pchData) + sizeof(pchMessageStart);
                            break;
                        }
                        nPos += ((unsigned char*)nFind - pchData) + 1;
                    }
                    else
                        nPos += sizeof(pchData) - sizeof(pchMessageStart) + 1;
                } while(!fRequestShutdown);
                if (nPos == (unsigned int)-1)
                    break;
                fseek(blkdat, nPos, SEEK_SET);
                unsigned int nSize;
                blkdat >> nSize;
                if (nSize > 0 && nSize <= MAX_BLOCK_SIZE)
                {
                    CBlock block;
                    blkdat >> block;
                    if (ProcessBlock(NULL,&block,false))
                    {
                        nLoaded++;
                        nPos += 4 + nSize;
                    }
                }
            }
        }
        catch (std::exception &e) {
            printf("%s() : Deserialize or I/O error caught during load\n",
                   __PRETTY_FUNCTION__);
        }
    }
    printf("Loaded %i blocks from external file in %" PRId64 "ms\n", nLoaded, GetTimeMillis() - nStart);
    return nLoaded > 0;
}

//////////////////////////////////////////////////////////////////////////////
//
// CAlert
//

extern map<uint256, CAlert> mapAlerts;
extern CCriticalSection cs_mapAlerts;

string GetWarnings(string strFor)
{
    int nPriority = 0;
    string strStatusBar;
    string strRPC;

    if (GetBoolArg("-testsafemode"))
        strRPC = "test";

    // Misc warnings like out of disk space and clock is wrong
    if (strMiscWarning != "")
    {
        nPriority = 1000;
        strStatusBar = strMiscWarning;
    }

    // if detected invalid checkpoint enter safe mode
    if (Checkpoints::hashInvalidCheckpoint != 0)
    {

		if (CHECKPOINT_DISTRIBUTED_MODE==1)
		{
			//10-18-2014-Halford- If invalid checkpoint found, reboot the node:
			printf("Moving Gridcoin into Checkpoint ADVISORY mode.\r\n");
			CheckpointsMode = Checkpoints::ADVISORY;
		}
		else
		{
			#if defined(WIN32) && defined(QT_GUI)
				int nResult = 0;
	    		std::string rebootme = "";
				if (mapArgs.count("-reboot"))
				{
					rebootme = GetArg("-reboot", "false");
				}
				if (rebootme == "true")
				{
					nResult = RebootClient();
					printf("Rebooting %u",nResult);
				}
			#endif

			nPriority = 3000;
			strStatusBar = strRPC = _("WARNING: Invalid checkpoint found! Displayed transactions may not be correct! You may need to upgrade, or notify developers.");
			printf("WARNING: Invalid checkpoint found! Displayed transactions may not be correct! You may need to upgrade, or notify developers.");
		}


    }

    // Alerts
    {
        LOCK(cs_mapAlerts);
        BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
        {
            const CAlert& alert = item.second;
            if (alert.AppliesToMe() && alert.nPriority > nPriority)
            {
                nPriority = alert.nPriority;
                strStatusBar = alert.strStatusBar;
                if (nPriority > 1000)
                    strRPC = strStatusBar;
            }
        }
    }

    if (strFor == "statusbar")
        return strStatusBar;
    else if (strFor == "rpc")
        return strRPC;
    assert(!"GetWarnings() : invalid parameter");
    return "error";
}








//////////////////////////////////////////////////////////////////////////////
//
// Messages
//


bool static AlreadyHave(CTxDB& txdb, const CInv& inv)
{
    switch (inv.type)
    {
    case MSG_TX:
        {
        bool txInMap = false;
        txInMap = mempool.exists(inv.hash);
        return txInMap ||
               mapOrphanTransactions.count(inv.hash) ||
               txdb.ContainsTx(inv.hash);
        }

    case MSG_BLOCK:
        return mapBlockIndex.count(inv.hash) ||
               mapOrphanBlocks.count(inv.hash);
    }
    // Don't know what it is, just say we already got one
    return true;
}



std::string GetLastBlockGRCAddress()
{
    CBlock block;
    const CBlockIndex* pindexPrev = GetLastBlockIndex(pindexBest, true);
	block.ReadFromDisk(pindexPrev);
	std::string hb = "";
	if (block.vtx.size() > 0) hb = block.vtx[0].hashBoinc;
	MiningCPID bb = DeserializeBoincBlock(hb);
	return bb.GRCAddress;
}

bool AcidTest(std::string precommand, std::string acid, CNode* pfrom)
{
	std::vector<std::string> vCommand = split(acid,",");
	if (vCommand.size() >= 6)
	{
		std::string sboinchashargs = DefaultOrgKey(12);  //Use 12 characters for inter-client communication
		std::string nonce =          vCommand[0];
		std::string command =        vCommand[1];
		std::string hash =           vCommand[2];
		std::string org =            vCommand[3];
		std::string pub_key_prefix = vCommand[4];
		std::string bhrn =           vCommand[5];
		std::string grid_pass =      vCommand[6];
		std::string grid_pass_decrypted = AdvancedDecryptWithSalt(grid_pass,sboinchashargs);

		if (grid_pass_decrypted != bhrn+nonce+org+pub_key_prefix)
		{
			if (fDebug10) printf("Decrypted gridpass %s <> hashed message",grid_pass_decrypted.c_str());
			nonce="";
			command="";
		}

		std::string pw1 = RetrieveMd5(nonce+","+command+","+org+","+pub_key_prefix+","+sboinchashargs);

		if (precommand=="aries")
		{
			//pfrom->securityversion = pw1;
		}
		if (fDebug10) printf(" Nonce %s,comm %s,hash %s,pw1 %s \r\n",nonce.c_str(),command.c_str(),hash.c_str(),pw1.c_str());
		//If timestamp too old; disconnect
		double timediff = std::abs(GetAdjustedTime() - cdbl(nonce,0));
	
		if (false && hash != pw1)
		{
			//2/16 18:06:48 Acid test failed for 192.168.1.4:32749 1478973994,encrypt,1b089d19d23fbc911c6967b948dd8324,windows			if (fDebug) printf("Acid test failed for %s %s.",NodeAddress(pfrom).c_str(),acid.c_str());
			double punishment = GetArg("-punishment", 10);
			pfrom->Misbehaving(punishment);
			return false;
		}
		return true;
	}
	else
	{
		if (fDebug2) printf("Message corrupted. Node %s partially banned.",NodeAddress(pfrom).c_str());
		pfrom->Misbehaving(1);
		return false;
	}
	return true;
}




// The message start string is designed to be unlikely to occur in normal data.
// The characters are rarely used upper ASCII, not valid as UTF-8, and produce
// a large 4-byte int at any alignment.
unsigned char pchMessageStart[4] = { 0x70, 0x35, 0x22, 0x05 };


std::string NodeAddress(CNode* pfrom)
{
	std::string ip = pfrom->addr.ToString();
	return ip;
}

double ExtractMagnitudeFromExplainMagnitude()
{
	    if (msNeuralResponse.empty()) return 0;
		try
		{
			std::vector<std::string> vMag = split(msNeuralResponse.c_str(),"<ROW>");
			for (unsigned int i = 0; i < vMag.size(); i++)
			{
				if (Contains(vMag[i],"Total Mag:"))
				{
					std::vector<std::string> vMyMag = split(vMag[i].c_str(),":");
					if (vMyMag.size() > 0)
					{
						std::string sSubMag = vMyMag[1];
						sSubMag = strReplace(sSubMag," ","");
						double dMag = cdbl("0"+sSubMag,0);
						return dMag;
					}
				}
			}
			return 0;
		}
		catch(...)
		{
			return 0;
		}
		return 0;
}

bool VerifyExplainMagnitudeResponse()
{
		if (msNeuralResponse.empty()) return false;
		try
		{
			double dMag = ExtractMagnitudeFromExplainMagnitude();
			if (dMag==0)
			{
					WriteCache("maginvalid","invalid",RoundToString(cdbl("0"+ReadCache("maginvalid","invalid"),0),0),GetAdjustedTime());
					double failures = cdbl("0"+ReadCache("maginvalid","invalid"),0);
					if (failures < 10)
					{
						msNeuralResponse = "";
					}
			}
			else
			{
				return true;
			}
		}
		catch(...)
		{
			return false;
		}
		return false;
}


bool SecurityTest(CNode* pfrom, bool acid_test)
{
	if (pfrom->nStartingHeight > (nBestHeight*.5) && acid_test) return true;
	return false;
}


bool PreventCommandAbuse(std::string sNeuralRequestID, std::string sCommandName)
{
				bool bIgnore = false;
				if (cdbl("0"+ReadCache(sCommandName,sNeuralRequestID),0) > 10)
				{
					if (fDebug10) printf("Ignoring %s request for %s",sCommandName.c_str(),sNeuralRequestID.c_str());
					bIgnore = true;
				}
				if (!bIgnore)
				{
					WriteCache(sCommandName,sNeuralRequestID,RoundToString(cdbl("0"+ReadCache(sCommandName,sNeuralRequestID),0),0),GetAdjustedTime());
				}
				return bIgnore;
}

bool static ProcessMessage(CNode* pfrom, string strCommand, CDataStream& vRecv, int64_t nTimeReceived)
{
    static map<CService, CPubKey> mapReuseKey;
    RandAddSeedPerfmon();
    if (fDebug10)
        printf("received: %s (%" PRIszu " bytes)\n", strCommand.c_str(), vRecv.size());
    if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
    {
        printf("dropmessagestest DROPPING RECV MESSAGE\n");
        return true;
    }

	// Stay in Sync - 8-9-2016
	if (!IsLockTimeWithinMinutes(nBootup,15))
	{
		if ((!IsLockTimeWithinMinutes(nLastAskedForBlocks,5) && WalletOutOfSync()) || (WalletOutOfSync() && fTestNet))
		{
			printf("\r\nBootup\r\n");
			AskForOutstandingBlocks(uint256(0));
		}
	}

	// Message Attacks ////////////////////////////////////////////////////////
	std::string precommand = strCommand;
	///////////////////////////////////////////////////////////////////////////

    if (strCommand == "aries")
    {
        // Each connection can only send one version message
        if (pfrom->nVersion != 0)
        {
            pfrom->Misbehaving(10);
            return false;
        }

        int64_t nTime;
        CAddress addrMe;
        CAddress addrFrom;
        uint64_t nNonce = 1;
		std::string acid = "";
		vRecv >> pfrom->nVersion >> pfrom->boinchashnonce >> pfrom->boinchashpw >> pfrom->cpid >> pfrom->enccpid >> acid >> pfrom->nServices >> nTime >> addrMe;

		
		//Halford - 12-26-2014 - Thwart Hackers
		bool ver_valid = AcidTest(strCommand,acid,pfrom);
        if (fDebug10) printf("Ver Acid %s, Validity %s ",acid.c_str(),YesNo(ver_valid).c_str());
		if (!ver_valid)
		{
		    pfrom->Misbehaving(100);
		    pfrom->fDisconnect = true;
            return false;
		}

		bool unauthorized = false;
		double timedrift = std::abs(GetAdjustedTime() - nTime);

		if (true)
		{
			if (timedrift > (8*60))
			{
				if (fDebug10) printf("Disconnecting unauthorized peer with Network Time so far off by %f seconds!\r\n",(double)timedrift);
				unauthorized = true;
			}
		}
		else
		{
			if (timedrift > (10*60) && LessVerbose(500))
			{
				if (fDebug10) printf("Disconnecting authorized peer with Network Time so far off by %f seconds!\r\n",(double)timedrift);
				unauthorized = true;
			}
		}

		if (unauthorized)
		{
			if (fDebug10) printf("  Disconnected unauthorized peer.         ");
            pfrom->Misbehaving(100);
		    pfrom->fDisconnect = true;
            return false;
        }


		// Ensure testnet users are running latest version as of 12-3-2015 (works in conjunction with block spamming)
		if (pfrom->nVersion < 180321 && fTestNet)
		{
		    // disconnect from peers older than this proto version
            if (fDebug10) printf("Testnet partner %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
            pfrom->fDisconnect = true;
            return false;
        }

        if (pfrom->nVersion < MIN_PEER_PROTO_VERSION)
        {
            // disconnect from peers older than this proto version
            if (fDebug10) printf("partner %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
            pfrom->fDisconnect = true;
            return false;
        }

		if (pfrom->nVersion < 180323 && !fTestNet && pindexBest->nHeight > 860500)
        {
            // disconnect from peers older than this proto version - Enforce Beacon Age - 3-26-2017
            if (fDebug10) printf("partner %s using obsolete version %i (before enforcing beacon age); disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
            pfrom->fDisconnect = true;
            return false;
        }

		if (!fTestNet && pfrom->nVersion < 180314 && IsResearchAgeEnabled(pindexBest->nHeight))
		{
		    // disconnect from peers older than this proto version
            if (fDebug10) printf("ResearchAge: partner %s using obsolete version %i; disconnecting\n", pfrom->addr.ToString().c_str(), pfrom->nVersion);
            pfrom->fDisconnect = true;
            return false;
       }

        if (pfrom->nVersion == 10300)
            pfrom->nVersion = 300;
        if (!vRecv.empty())
            vRecv >> addrFrom >> nNonce;
        if (!vRecv.empty())
            vRecv >> pfrom->strSubVer;

        if (!vRecv.empty())
            vRecv >> pfrom->nStartingHeight;
		// 12-5-2015 - Append Trust fields
		pfrom->nTrust = 0;
		
		if (!vRecv.empty())			vRecv >> pfrom->sGRCAddress;
		
		
		// Allow newbies to connect easily with 0 blocks
		if (GetArgument("autoban","true") == "true")
		{
				
				// Note: Hacking attempts start in this area
				if (false && pfrom->nStartingHeight < (nBestHeight/2) && LessVerbose(1) && !fTestNet)
				{
					if (fDebug3) printf("Node with low height");
					pfrom->fDisconnect=true;
					return false;
				}
				/*
				
				if (pfrom->nStartingHeight < 1 && LessVerbose(980) && !fTestNet)
				{
					pfrom->Misbehaving(100);
					if (fDebug3) printf("Disconnecting possible hacker node.  Banned for 24 hours.\r\n");
			    	pfrom->fDisconnect=true;
					return false;
				}
				*/


				// End of critical Section

				if (pfrom->nStartingHeight < 1 && pfrom->nServices == 0 )
				{
					pfrom->Misbehaving(100);
					if (fDebug3) printf("Disconnecting possible hacker node with no services.  Banned for 24 hours.\r\n");
			    	pfrom->fDisconnect=true;
					return false;
				}
		}

	

		if (pfrom->fInbound && addrMe.IsRoutable())
        {
            pfrom->addrLocal = addrMe;
            SeenLocal(addrMe);
        }

        // Disconnect if we connected to ourself
        if (nNonce == nLocalHostNonce && nNonce > 1)
        {
            if (fDebug3) printf("connected to self at %s, disconnecting\n", pfrom->addr.ToString().c_str());
            pfrom->fDisconnect = true;
            return true;
        }

        // record my external IP reported by peer
        if (addrFrom.IsRoutable() && addrMe.IsRoutable())
            addrSeenByPeer = addrMe;

        // Be shy and don't send version until we hear
        if (pfrom->fInbound)
            pfrom->PushVersion();

        pfrom->fClient = !(pfrom->nServices & NODE_NETWORK);

        if (GetBoolArg("-synctime", true))
            AddTimeData(pfrom->addr, nTime);

        // Change version
        pfrom->PushMessage("verack");
        pfrom->ssSend.SetVersion(min(pfrom->nVersion, PROTOCOL_VERSION));

			
        if (!pfrom->fInbound)
        {
            // Advertise our address
            if (!fNoListen && !IsInitialBlockDownload())
            {
                CAddress addr = GetLocalAddress(&pfrom->addr);
                if (addr.IsRoutable())
                    pfrom->PushAddress(addr);
            }

            // Get recent addresses
            if (pfrom->fOneShot || pfrom->nVersion >= CADDR_TIME_VERSION || addrman.size() < 1000)
            {
                pfrom->PushMessage("getaddr");
                pfrom->fGetAddr = true;
            }
            addrman.Good(pfrom->addr);
        }
		else
		{
            if (((CNetAddr)pfrom->addr) == (CNetAddr)addrFrom)
            {
				if (SecurityTest(pfrom,ver_valid))
				{
					//Dont store the peer unless it passes the test
					addrman.Add(addrFrom, addrFrom);
	                addrman.Good(addrFrom);
				}
            }
        }

    
 	    // Ask the first connected node for block updates
        static int nAskedForBlocks = 0;
        if (!pfrom->fClient && !pfrom->fOneShot &&
            (pfrom->nStartingHeight > (nBestHeight - 144)) &&
            (pfrom->nVersion < NOBLKS_VERSION_START ||
             pfrom->nVersion >= NOBLKS_VERSION_END) &&
             (nAskedForBlocks < 1 || (vNodes.size() <= 1 && nAskedForBlocks < 1)))
        {
            nAskedForBlocks++;
            pfrom->PushGetBlocks(pindexBest, uint256(0), true);
			if (fDebug3) printf("\r\nAsked For blocks.\r\n");
        }

        // Relay alerts
        {
            LOCK(cs_mapAlerts);
            BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
                item.second.RelayTo(pfrom);
        }

        // Relay sync-checkpoint
        {
            LOCK(Checkpoints::cs_hashSyncCheckpoint);
            if (!Checkpoints::checkpointMessage.IsNull())
                Checkpoints::checkpointMessage.RelayTo(pfrom);
        }

        pfrom->fSuccessfullyConnected = true;

        if (fDebug10) printf("receive version message: version %d, blocks=%d, us=%s, them=%s, peer=%s\n", pfrom->nVersion,
			pfrom->nStartingHeight, addrMe.ToString().c_str(), addrFrom.ToString().c_str(), pfrom->addr.ToString().c_str());

        cPeerBlockCounts.input(pfrom->nStartingHeight);

        // ppcoin: ask for pending sync-checkpoint if any
        if (!IsInitialBlockDownload())
            Checkpoints::AskForPendingSyncCheckpoint(pfrom);
    }
    else if (pfrom->nVersion == 0)
    {
        // Must have a version message before anything else 1-10-2015 Halford
		printf("Hack attempt from %s - %s (banned) \r\n",pfrom->addrName.c_str(),NodeAddress(pfrom).c_str());
        pfrom->Misbehaving(100);
		pfrom->fDisconnect=true;
        return false;
    }
    else if (strCommand == "verack")
    {
        pfrom->SetRecvVersion(min(pfrom->nVersion, PROTOCOL_VERSION));
    }
    else if (strCommand == "gridaddr")
    {
		//addr->gridaddr
        vector<CAddress> vAddr;
        vRecv >> vAddr;

        // Don't want addr from older versions unless seeding
        if (pfrom->nVersion < CADDR_TIME_VERSION && addrman.size() > 1000)
            return true;
        if (vAddr.size() > 1000)
        {
            pfrom->Misbehaving(10);
            return error("message addr size() = %" PRIszu "", vAddr.size());
        }

		// Don't store the node address unless they have block height > 50%
		if (pfrom->nStartingHeight < (nBestHeight*.5) && LessVerbose(975)) return true;

        // Store the new addresses
        vector<CAddress> vAddrOk;
        int64_t nNow = GetAdjustedTime();
        int64_t nSince = nNow - 10 * 60;
        BOOST_FOREACH(CAddress& addr, vAddr)
        {
            if (fShutdown)
                return true;
            if (addr.nTime <= 100000000 || addr.nTime > nNow + 10 * 60)
                addr.nTime = nNow - 5 * 24 * 60 * 60;
            pfrom->AddAddressKnown(addr);
            bool fReachable = IsReachable(addr);

			bool bad_node = (pfrom->nStartingHeight < 1 && LessVerbose(700));


            if (addr.nTime > nSince && !pfrom->fGetAddr && vAddr.size() <= 10 && addr.IsRoutable() && !bad_node)
            {
                // Relay to a limited number of other nodes
                {
                    LOCK(cs_vNodes);
                    // Use deterministic randomness to send to the same nodes for 24 hours
                    // at a time so the setAddrKnowns of the chosen nodes prevent repeats
                    static uint256 hashSalt;
                    if (hashSalt == 0)
                        hashSalt = GetRandHash();
                    uint64_t hashAddr = addr.GetHash();
                    uint256 hashRand = hashSalt ^ (hashAddr<<32) ^ (( GetAdjustedTime() +hashAddr)/(24*60*60));
                    hashRand = Hash(BEGIN(hashRand), END(hashRand));
                    multimap<uint256, CNode*> mapMix;
                    BOOST_FOREACH(CNode* pnode, vNodes)
                    {
                        if (pnode->nVersion < CADDR_TIME_VERSION)
                            continue;
                        unsigned int nPointer;
                        memcpy(&nPointer, &pnode, sizeof(nPointer));
                        uint256 hashKey = hashRand ^ nPointer;
                        hashKey = Hash(BEGIN(hashKey), END(hashKey));
                        mapMix.insert(make_pair(hashKey, pnode));
                    }
                    int nRelayNodes = fReachable ? 2 : 1; // limited relaying of addresses outside our network(s)
                    for (multimap<uint256, CNode*>::iterator mi = mapMix.begin(); mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
                        ((*mi).second)->PushAddress(addr);
                }
            }
            // Do not store addresses outside our network
            if (fReachable)
                vAddrOk.push_back(addr);
        }
        addrman.Add(vAddrOk, pfrom->addr, 2 * 60 * 60);
        if (vAddr.size() < 1000)
            pfrom->fGetAddr = false;
        if (pfrom->fOneShot)
            pfrom->fDisconnect = true;
    }

    else if (strCommand == "inv")
    {
        vector<CInv> vInv;
        vRecv >> vInv;
        if (vInv.size() > MAX_INV_SZ)
        {
            pfrom->Misbehaving(50);
			printf("\r\n **Hacker tried to send inventory > MAX_INV_SZ **\r\n");
            return error("message inv size() = %" PRIszu "", vInv.size());
        }

        // find last block in inv vector
        unsigned int nLastBlock = (unsigned int)(-1);
        for (unsigned int nInv = 0; nInv < vInv.size(); nInv++) {
            if (vInv[vInv.size() - 1 - nInv].type == MSG_BLOCK) {
                nLastBlock = vInv.size() - 1 - nInv;
                break;
            }
        }
        CTxDB txdb("r");
        for (unsigned int nInv = 0; nInv < vInv.size(); nInv++)
        {
            const CInv &inv = vInv[nInv];

            if (fShutdown)
                return true;
            pfrom->AddInventoryKnown(inv);

            bool fAlreadyHave = AlreadyHave(txdb, inv);
            if (fDebug10)
                printf("  got inventory: %s  %s\n", inv.ToString().c_str(), fAlreadyHave ? "have" : "new");

            if (!fAlreadyHave)
                pfrom->AskFor(inv);
            else if (inv.type == MSG_BLOCK && mapOrphanBlocks.count(inv.hash)) {
                pfrom->PushGetBlocks(pindexBest, GetOrphanRoot(mapOrphanBlocks[inv.hash]), true);
            } else if (nInv == nLastBlock) {
                // In case we are on a very long side-chain, it is possible that we already have
                // the last block in an inv bundle sent in response to getblocks. Try to detect
                // this situation and push another getblocks to continue.
                pfrom->PushGetBlocks(mapBlockIndex[inv.hash], uint256(0), true);
                if (fDebug10)
                    printf("force getblock request: %s\n", inv.ToString().c_str());
            }

            // Track requests for our stuff
            Inventory(inv.hash);
        }
    }


    else if (strCommand == "getdata")
    {
        vector<CInv> vInv;
        vRecv >> vInv;
        if (vInv.size() > MAX_INV_SZ)
        {
            pfrom->Misbehaving(10);
            return error("message getdata size() = %" PRIszu "", vInv.size());
        }

        if (fDebugNet || (vInv.size() != 1))
		{
            if (fDebug10)  printf("received getdata (%" PRIszu " invsz)\n", vInv.size());
		}

        BOOST_FOREACH(const CInv& inv, vInv)
        {
            if (fShutdown)
                return true;
            if (fDebugNet || (vInv.size() == 1))
			{
              if (fDebug10)   printf("received getdata for: %s\n", inv.ToString().c_str());
			}

            if (inv.type == MSG_BLOCK)
            {
                // Send block from disk
                map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(inv.hash);
                if (mi != mapBlockIndex.end())
                {
                    CBlock block;
                    block.ReadFromDisk((*mi).second);
					//HALFORD 12-26-2014
					std::string acid = GetCommandNonce("encrypt");
                    pfrom->PushMessage("encrypt", block, acid);

                    // Trigger them to send a getblocks request for the next batch of inventory
                    if (inv.hash == pfrom->hashContinue)
                    {
                        // ppcoin: send latest proof-of-work block to allow the
                        // download node to accept as orphan (proof-of-stake
                        // block might be rejected by stake connection check)
                        vector<CInv> vInv;
                        vInv.push_back(CInv(MSG_BLOCK, GetLastBlockIndex(pindexBest, false)->GetBlockHash()));
                        pfrom->PushMessage("inv", vInv);
                        pfrom->hashContinue = 0;
                    }
                }
            }
             else if (inv.IsKnownType())
            {
                // Send stream from relay memory
                bool pushed = false;
                {
                    LOCK(cs_mapRelay);
                    map<CInv, CDataStream>::iterator mi = mapRelay.find(inv);
                    if (mi != mapRelay.end()) {
                        pfrom->PushMessage(inv.GetCommand(), (*mi).second);
                        pushed = true;
                    }
                }
                if (!pushed && inv.type == MSG_TX) {
                    CTransaction tx;
                    if (mempool.lookup(inv.hash, tx)) {
                        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
                        ss.reserve(1000);
                        ss << tx;
                        pfrom->PushMessage("tx", ss);
                    }
                }
            }

            // Track requests for our stuff
            Inventory(inv.hash);
        }
    }

	else if (strCommand == "getblocks")
    {
        CBlockLocator locator;
        uint256 hashStop;
        vRecv >> locator >> hashStop;

        // Find the last block the caller has in the main chain
        CBlockIndex* pindex = locator.GetBlockIndex();

        // Send the rest of the chain
        if (pindex)
            pindex = pindex->pnext;
        int nLimit = 1000;

        if (fDebug3) printf("\r\ngetblocks %d to %s limit %d\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str(), nLimit);
        for (; pindex; pindex = pindex->pnext)
        {
            if (pindex->GetBlockHash() == hashStop)
            {
                if (fDebug3) printf("\r\n  getblocks stopping at %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());
                // ppcoin: tell downloading node about the latest block if it's
                // without risk being rejected due to stake connection check
                if (hashStop != hashBestChain && pindex->GetBlockTime() + nStakeMinAge > pindexBest->GetBlockTime())
                    pfrom->PushInventory(CInv(MSG_BLOCK, hashBestChain));
                break;
            }
            pfrom->PushInventory(CInv(MSG_BLOCK, pindex->GetBlockHash()));
            if (--nLimit <= 0)
            {
                // When this block is requested, we'll send an inv that'll make them
                // getblocks the next batch of inventory.
                if (fDebug3) printf("\r\n  getblocks stopping at limit %d %s\n", pindex->nHeight, pindex->GetBlockHash().ToString().substr(0,20).c_str());
                pfrom->hashContinue = pindex->GetBlockHash();
                break;
            }
        }
    }
    else if (strCommand == "checkpoint")
    {
        CSyncCheckpoint checkpoint;
        vRecv >> checkpoint;
		//Checkpoint received from node with more than 1 Million GRC:
		if (CHECKPOINT_DISTRIBUTED_MODE==0 || CHECKPOINT_DISTRIBUTED_MODE==1)
		{
			if (checkpoint.ProcessSyncCheckpoint(pfrom))
			{
				// Relay
				pfrom->hashCheckpointKnown = checkpoint.hashCheckpoint;
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodes)
					checkpoint.RelayTo(pnode);
			}
		}
		else if (CHECKPOINT_DISTRIBUTED_MODE == 2)
		{
			// R HALFORD: One of our global GRC nodes solved a PoR block, store the last blockhash in memory
			muGlobalCheckpointHash = checkpoint.hashCheckpointGlobal;
			muGlobalCheckpointHashCounter=0;
			// Relay
			pfrom->hashCheckpointKnown = checkpoint.hashCheckpointGlobal;
			//Prevent broadcast storm: If not broadcast yet, relay the checkpoint globally:
			if (muGlobalCheckpointHashRelayed != checkpoint.hashCheckpointGlobal && checkpoint.hashCheckpointGlobal != 0)
			{
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodes)
				{
					checkpoint.RelayTo(pnode);
				}
			}
		}
    }

    else if (strCommand == "getheaders")
    {
        CBlockLocator locator;
        uint256 hashStop;
        vRecv >> locator >> hashStop;

        CBlockIndex* pindex = NULL;
        if (locator.IsNull())
        {
            // If locator is null, return the hashStop block
            map<uint256, CBlockIndex*>::iterator mi = mapBlockIndex.find(hashStop);
            if (mi == mapBlockIndex.end())
                return true;
            pindex = (*mi).second;
        }
        else
        {
            // Find the last block the caller has in the main chain
            pindex = locator.GetBlockIndex();
            if (pindex)
                pindex = pindex->pnext;
        }

        vector<CBlock> vHeaders;
        int nLimit = 1000;
        printf("\r\ngetheaders %d to %s\n", (pindex ? pindex->nHeight : -1), hashStop.ToString().substr(0,20).c_str());
        for (; pindex; pindex = pindex->pnext)
        {
            vHeaders.push_back(pindex->GetBlockHeader());
            if (--nLimit <= 0 || pindex->GetBlockHash() == hashStop)
                break;
        }
        pfrom->PushMessage("headers", vHeaders);
    }
    else if (strCommand == "tx")
    {
        vector<uint256> vWorkQueue;
        vector<uint256> vEraseQueue;
        CTransaction tx;
        vRecv >> tx;

        CInv inv(MSG_TX, tx.GetHash());
        pfrom->AddInventoryKnown(inv);

        bool fMissingInputs = false;
        if (AcceptToMemoryPool(mempool, tx, &fMissingInputs))
        {
            RelayTransaction(tx, inv.hash);
            mapAlreadyAskedFor.erase(inv);
            vWorkQueue.push_back(inv.hash);
            vEraseQueue.push_back(inv.hash);
         
			// Recursively process any orphan transactions that depended on this one
            for (unsigned int i = 0; i < vWorkQueue.size(); i++)
            {
                uint256 hashPrev = vWorkQueue[i];
                for (set<uint256>::iterator mi = mapOrphanTransactionsByPrev[hashPrev].begin();
                     mi != mapOrphanTransactionsByPrev[hashPrev].end();
                     ++mi)
                {
                    const uint256& orphanTxHash = *mi;
                    CTransaction& orphanTx = mapOrphanTransactions[orphanTxHash];
                    bool fMissingInputs2 = false;

                    if (AcceptToMemoryPool(mempool, orphanTx, &fMissingInputs2))
                    {
                        printf("   accepted orphan tx %s\n", orphanTxHash.ToString().substr(0,10).c_str());
                        RelayTransaction(orphanTx, orphanTxHash);
                        mapAlreadyAskedFor.erase(CInv(MSG_TX, orphanTxHash));
                        vWorkQueue.push_back(orphanTxHash);
                        vEraseQueue.push_back(orphanTxHash);
						pfrom->nTrust++;
                    }
                    else if (!fMissingInputs2)
                    {
                        // invalid orphan
                        vEraseQueue.push_back(orphanTxHash);
                        printf("   removed invalid orphan tx %s\n", orphanTxHash.ToString().substr(0,10).c_str());
                    }
                }
            }

            BOOST_FOREACH(uint256 hash, vEraseQueue)
                EraseOrphanTx(hash);
        }
        else if (fMissingInputs)
        {
            AddOrphanTx(tx);

            // DoS prevention: do not allow mapOrphanTransactions to grow unbounded
            unsigned int nEvicted = LimitOrphanTxSize(MAX_ORPHAN_TRANSACTIONS);
            if (nEvicted > 0)
                printf("mapOrphan overflow, removed %u tx\n", nEvicted);
        }
        if (tx.nDoS) pfrom->Misbehaving(tx.nDoS);
    }


    else if (strCommand == "encrypt")
    {
		//Response from getblocks, message = block

        CBlock block;
		std::string acid = "";
        vRecv >> block >> acid;
        uint256 hashBlock = block.GetHash();

		bool block_valid = AcidTest(strCommand,acid,pfrom);
		if (!block_valid) 
		{	
			printf("\r\n Acid test failed for block %s \r\n",hashBlock.ToString().c_str());
			return false;
		}

		if (fDebug10) printf("Acid %s, Validity %s ",acid.c_str(),YesNo(block_valid).c_str());

        printf(" Received block %s; ", hashBlock.ToString().c_str());
        if (fDebug10) block.print();

        CInv inv(MSG_BLOCK, hashBlock);
        pfrom->AddInventoryKnown(inv);

        if (ProcessBlock(pfrom, &block, false))
		{
	        mapAlreadyAskedFor.erase(inv);
			pfrom->nTrust++;
		}
        if (block.nDoS) 
		{
				pfrom->Misbehaving(block.nDoS);
				pfrom->nTrust--;
		}

    }


    else if (strCommand == "getaddr")
    {
        // Don't return addresses older than nCutOff timestamp
        int64_t nCutOff =  GetAdjustedTime() - (nNodeLifespan * 24 * 60 * 60);
        pfrom->vAddrToSend.clear();
        vector<CAddress> vAddr = addrman.GetAddr();
        BOOST_FOREACH(const CAddress &addr, vAddr)
            if(addr.nTime > nCutOff)
                pfrom->PushAddress(addr);
    }


    else if (strCommand == "mempool")
    {
        std::vector<uint256> vtxid;
        mempool.queryHashes(vtxid);
        vector<CInv> vInv;
        for (unsigned int i = 0; i < vtxid.size(); i++) {
            CInv inv(MSG_TX, vtxid[i]);
            vInv.push_back(inv);
            if (i == (MAX_INV_SZ - 1))
                    break;
        }
        if (vInv.size() > 0)
            pfrom->PushMessage("inv", vInv);
    }

    else if (strCommand == "reply")
    {
        uint256 hashReply;
        vRecv >> hashReply;

        CRequestTracker tracker;
        {
            LOCK(pfrom->cs_mapRequests);
            map<uint256, CRequestTracker>::iterator mi = pfrom->mapRequests.find(hashReply);
            if (mi != pfrom->mapRequests.end())
            {
                tracker = (*mi).second;
                pfrom->mapRequests.erase(mi);
            }
        }
        if (!tracker.IsNull())
            tracker.fn(tracker.param1, vRecv);
    }
	else if (strCommand == "neural")
	{
 	 	    //printf("Received Neural Request \r\n");

			std::string neural_request = "";
			std::string neural_request_id = "";
	        vRecv >> neural_request >> neural_request_id;  // foreign node issued neural request with request ID:
			//printf("neural request %s \r\n",neural_request.c_str());
			std::string neural_response = "generic_response";

			if (neural_request=="neural_data")
			{
				if (!PreventCommandAbuse("neural_data",NodeAddress(pfrom)))
				{
					std::string contract = "";
					#if defined(WIN32) && defined(QT_GUI)
							std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
							qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
							contract = qtGetNeuralContract("");
					#endif
					pfrom->PushMessage("ndata_nresp", contract);
				}
			}
			else if (neural_request=="neural_hash")
			{
				#if defined(WIN32) && defined(QT_GUI)
					neural_response = qtGetNeuralHash("");
				#endif
				//printf("Neural response %s",neural_response.c_str());
	            pfrom->PushMessage("hash_nresp", neural_response);
			}
			else if (neural_request=="explainmag")
			{
				// To prevent abuse, only respond to a certain amount of explainmag requests per day per cpid
				bool bIgnore = false;
				if (cdbl("0"+ReadCache("explainmag",neural_request_id),0) > 10)
				{
					if (fDebug10) printf("Ignoring explainmag request for %s",neural_request_id.c_str());
		 			pfrom->Misbehaving(1);
					bIgnore = true;
				}
				if (!bIgnore)
				{
					WriteCache("explainmag",neural_request_id,RoundToString(cdbl("0"+ReadCache("explainmag",neural_request_id),0),0),GetAdjustedTime());
					// 7/11/2015 - Allow linux/mac to make neural requests
					#if defined(WIN32) && defined(QT_GUI)
						neural_response = qtExecuteDotNetStringFunction("ExplainMag",neural_request_id);
					#endif
				    pfrom->PushMessage("expmag_nresp", neural_response);
				}
			}
			else if (neural_request=="addbeacon")
			{
				    std::string sBeacon_Sponsorship_Enabled = GetArgument("sponsor", "false");
					if (sBeacon_Sponsorship_Enabled=="true")
					{

						std::vector<std::string> s = split(neural_request_id,"|");
						std::string result = "Malformed Beacon";
						bool bIgnore = false;

						if (s.size() > 1)
						{
								std::string cpid = s[0];
								std::string myBeacon = MyBeaconExists(cpid);
								if (myBeacon.length() > 10)
								{
									bIgnore=true;
									result = "Beacon already exists; ignoring request.";
									if (fDebug10) printf("Add neural beacon: %s ",result.c_str());
		  							pfrom->Misbehaving(10);
								}
								if (s.size() >= 3)
								{
									std::string cpidv2 = s[2];
									std::string hashRand = s[3];
									uint256 uHash(hashRand);
									bool IsCPIDValid2 = CPID_IsCPIDValid(cpid,cpidv2,uHash);
									if (!IsCPIDValid2)
									{
										result = "Unable to sponsor beacon for invalid CPID " + cpid;
										if (fDebug10) printf("Add Neural Beacon: %s",result.c_str());
		  								pfrom->Misbehaving(10);
     									bIgnore = true;
									}
								}
								// Have we already sponsored beacon before?
								if (ReadCache("sponsored",cpid)=="true")
								{
									result = "Unable to sponsor beacon for CPID " + cpid + ": already sponsored previously.";
									if (fDebug10) printf("Add neural beacon : %s",result.c_str());
		 							pfrom->Misbehaving(10);
									bIgnore = true;
								}
								if (!bIgnore)
								{
										result = AddContract("beacon",cpid,s[1]);
										WriteCache("sponsored",cpid,"true",GetAdjustedTime());
								}
								if (fDebug3) printf("Acting as Sponsor for CPID %s : adding beacon %s; result %s", cpid.c_str(), s[1].c_str(), result.c_str());
						}
						pfrom->PushMessage("addbeac_nresp", result);
					}
			}
			else if (neural_request=="quorum")
			{
				// 7-12-2015 Resolve discrepencies in the neural network intelligently - allow nodes to speak to each other
				std::string contract = "";
				#if defined(WIN32) && defined(QT_GUI)
						std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
						qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
						contract = qtGetNeuralContract("");
				#endif
				//if (fDebug10) printf("Quorum response %f \r\n",(double)contract.length());
	            pfrom->PushMessage("quorum_nresp", contract);
			}
			else
			{
				neural_response="generic_response";
			}

	}
    else if (strCommand == "ping")
    {


		std::string acid = "";
        if (pfrom->nVersion > BIP0031_VERSION)
        {
            uint64_t nonce = 0;
            vRecv >> nonce >> acid;
			bool pong_valid = AcidTest(strCommand,acid,pfrom);
			if (!pong_valid) return false;
			//if (fDebug10) printf("pong valid %s",YesNo(pong_valid).c_str());

            // Echo the message back with the nonce. This allows for two useful features:
            //
            // 1) A remote node can quickly check if the connection is operational
            // 2) Remote nodes can measure the latency of the network thread. If this node
            //    is overloaded it won't respond to pings quickly and the remote node can
            //    avoid sending us more work, like chain download requests.
            //
            // The nonce stops the remote getting confused between different pings: without
            // it, if the remote node sends a ping once per second and this node takes 5
            // seconds to respond to each, the 5th ping the remote sends would appear to
            // return very quickly.
			pfrom->PushMessage("pong", nonce);
        }
	}
	else if (strCommand == "pong")
    {
        int64_t pingUsecEnd = GetTimeMicros();
        uint64_t nonce = 0;
        size_t nAvail = vRecv.in_avail();
        bool bPingFinished = false;
        std::string sProblem;

        if (nAvail >= sizeof(nonce)) {
            vRecv >> nonce;

            // Only process pong message if there is an outstanding ping (old ping without nonce should never pong)
            if (pfrom->nPingNonceSent != 0) 
			{
                if (nonce == pfrom->nPingNonceSent) 
				{
                    // Matching pong received, this ping is no longer outstanding
                    bPingFinished = true;
                    int64_t pingUsecTime = pingUsecEnd - pfrom->nPingUsecStart;
                    if (pingUsecTime > 0) {
                        // Successful ping time measurement, replace previous
                        pfrom->nPingUsecTime = pingUsecTime;
                    } else {
                        // This should never happen
                        sProblem = "Timing mishap";
                    }
                } else {
                    // Nonce mismatches are normal when pings are overlapping
                    sProblem = "Nonce mismatch";
                    if (nonce == 0) {
                        // This is most likely a bug in another implementation somewhere, cancel this ping
                        bPingFinished = true;
                        sProblem = "Nonce zero";
                    }
                }
            } else {
                sProblem = "Unsolicited pong without ping";
            }
        } else {
            // This is most likely a bug in another implementation somewhere, cancel this ping
            bPingFinished = true;
            sProblem = "Short payload";
        }

        if (!(sProblem.empty())) {
            printf("pong %s %s: %s, %" PRIx64 " expected, %" PRIx64 " received, %f bytes\n"
                , pfrom->addr.ToString().c_str()
                , pfrom->strSubVer.c_str()
                , sProblem.c_str()                , pfrom->nPingNonceSent                , nonce                , (double)nAvail);
        }
        if (bPingFinished) {
            pfrom->nPingNonceSent = 0;
        }
    }
	else if (strCommand=="addbeac_nresp")
	{
			std::string neural_response = "?";
	        vRecv >> neural_response;
			pfrom->NeuralHash = neural_response;
			if (fDebug3) printf("Add Beacon Neural Response %s \r\n",neural_response.c_str());
			msMiningErrors7=neural_response;
	}
	else if (strCommand == "hash_nresp")
	{
			std::string neural_response = "";
	        vRecv >> neural_response;
			// if (pfrom->nNeuralRequestSent != 0)
			// nNeuralNonce must match request ID
			pfrom->NeuralHash = neural_response;
			if (fDebug10) printf("hash_Neural Response %s \r\n",neural_response.c_str());
	}
	else if (strCommand == "expmag_nresp")
	{
			std::string neural_response = "";
	        vRecv >> neural_response;
			if (neural_response.length() > 10)
			{
				msNeuralResponse=neural_response;
				//If invalid, try again 10-20-2015
				VerifyExplainMagnitudeResponse();
			}
			if (fDebug10) printf("expmag_Neural Response %s \r\n",neural_response.c_str());
	}
	else if (strCommand == "quorum_nresp")
	{
			std::string neural_contract = "";
	        vRecv >> neural_contract;
			if (fDebug && neural_contract.length() > 100) printf("Quorum contract received %s",neural_contract.substr(0,80).c_str());
			if (neural_contract.length() > 10)
			{
				 std::string results = "";
				 //Resolve discrepancies
		 		 #if defined(WIN32) && defined(QT_GUI)
	 	 		 	std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
					qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
					results = qtExecuteDotNetStringFunction("ResolveDiscrepancies",neural_contract);
				 #endif
	  			 if (fDebug && !results.empty()) printf("Quorum Resolution: %s \r\n",results.c_str());
			}
	}
	else if (strCommand == "ndata_nresp")
	{
			std::string neural_contract = "";
	        vRecv >> neural_contract;
			if (fDebug3 && neural_contract.length() > 100) printf("Quorum contract received %s",neural_contract.substr(0,80).c_str());
			if (neural_contract.length() > 10)
			{
				 std::string results = "";
				 //Resolve discrepancies
		 		 #if defined(WIN32) && defined(QT_GUI)
	 	 		 	std::string testnet_flag = fTestNet ? "TESTNET" : "MAINNET";
					qtExecuteGenericFunction("SetTestNetFlag",testnet_flag);
					printf("\r\n** Sync neural network data from supermajority **\r\n");
					results = qtExecuteDotNetStringFunction("ResolveCurrentDiscrepancies",neural_contract);
				 #endif
	  			 if (fDebug && !results.empty()) printf("Quorum Resolution: %s \r\n",results.c_str());
				 // Resume the full DPOR sync at this point now that we have the supermajority data
				 if (results=="SUCCESS")  FullSyncWithDPORNodes();
			}
	}

    else if (strCommand == "pong_old")
    {
        int64_t pingUsecEnd = nTimeReceived;
        uint64_t nonce = 0;
        size_t nAvail = vRecv.in_avail();
        bool bPingFinished = false;
        std::string sProblem;
	    if (nAvail >= sizeof(nonce)) {
            vRecv >> nonce;

            // Only process pong message if there is an outstanding ping (old ping without nonce should never pong)
            if (pfrom->nPingNonceSent != 0) {
                if (nonce == pfrom->nPingNonceSent) {
                    // Matching pong received, this ping is no longer outstanding
                    bPingFinished = true;
                    int64_t pingUsecTime = pingUsecEnd - pfrom->nPingUsecStart;
                    if (pingUsecTime >= -1) {
                        // Successful ping time measurement, replace previous
                        pfrom->nPingUsecTime = pingUsecTime;
                    } else {
                        // This should never happen
                        sProblem = "Timing mishap";
                    }
                } else {
                    // Nonce mismatches are normal when pings are overlapping
                    sProblem = "Nonce mismatch";
                    if (nonce == 0) {
                        // This is most likely a bug in another implementation somewhere, cancel this ping
                        bPingFinished = true;
                        sProblem = "Nonce zero";
                    }
                }
            } else {
                sProblem = "Unsolicited pong without ping";
            }
        } else {
            // This is most likely a bug in another implementation somewhere, cancel this ping
            bPingFinished = true;
            sProblem = "Short payload";
        }

        if (!(sProblem.empty())) {
            printf("pong %s %s: %s, %" PRIx64 " expected, %" PRIx64 " received, %f bytes\n"
                , pfrom->addr.ToString().c_str()
                , pfrom->strSubVer.c_str()
                , sProblem.c_str()
                , pfrom->nPingNonceSent
                , nonce
                , (double)nAvail);
        }
        if (bPingFinished) {
            pfrom->nPingNonceSent = 0;
        }
    }


    else if (strCommand == "alert")
    {
        CAlert alert;
        vRecv >> alert;

        uint256 alertHash = alert.GetHash();
        if (pfrom->setKnown.count(alertHash) == 0)
        {
            if (alert.ProcessAlert())
            {
                // Relay
                pfrom->setKnown.insert(alertHash);
                {
                    LOCK(cs_vNodes);
                    BOOST_FOREACH(CNode* pnode, vNodes)
                        alert.RelayTo(pnode);
                }
            }
            else {
                // Small DoS penalty so peers that send us lots of
                // duplicate/expired/invalid-signature/whatever alerts
                // eventually get banned.
                // This isn't a Misbehaving(100) (immediate ban) because the
                // peer might be an older or different implementation with
                // a different signature key, etc.
                pfrom->Misbehaving(10);
            }
        }
    }


    else
    {
        // Ignore unknown commands for extensibility
        // Let the peer know that we didn't find what it asked for, so it doesn't
        // have to wait around forever. Currently only SPV clients actually care
        // about this message: it's needed when they are recursively walking the
        // dependencies of relevant unconfirmed transactions. SPV clients want to
        // do that because they want to know about (and store and rebroadcast and
        // risk analyze) the dependencies of transactions relevant to them, without
        // having to download the entire memory pool.


    }

    // Update the last seen time for this node's address
    if (pfrom->fNetworkNode)
        if (strCommand == "aries" || strCommand == "gridaddr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
            AddressCurrentlyConnected(pfrom->addr);

    return true;
}


void AddPeek(std::string data)
{
	return;
	std::string buffer = RoundToString((double)GetAdjustedTime(),0) + ":" + data + "<CR>";
	msPeek += buffer;
	if (msPeek.length() > 60000) msPeek = "";
	if ((GetAdjustedTime() - nLastPeek) > 60)
	{
		if (fDebug) printf("\r\nLong Duration : %s\r\n",buffer.c_str());
	}
	nLastPeek = GetAdjustedTime();
}


// requires LOCK(cs_vRecvMsg)
bool ProcessMessages(CNode* pfrom)
{
    //
    // Message format
    //  (4) message start
    //  (12) command
    //  (4) size
    //  (4) checksum
    //  (x) data
    //
    bool fOk = true;

    std::deque<CNetMessage>::iterator it = pfrom->vRecvMsg.begin();
    while (!pfrom->fDisconnect && it != pfrom->vRecvMsg.end()) {
        // Don't bother if send buffer is too full to respond anyway
        if (pfrom->nSendSize >= SendBufferSize())
            break;

        // get next message
        CNetMessage& msg = *it;

        //if (fDebug10)
        //    printf("ProcessMessages(message %u msgsz, %zu bytes, complete:%s)\n",
        //            msg.hdr.nMessageSize, msg.vRecv.size(),
        //            msg.complete() ? "Y" : "N");

        // end, if an incomplete message is found
        if (!msg.complete())
            break;

        // at this point, any failure means we can delete the current message
        it++;

        // Scan for message start
        if (memcmp(msg.hdr.pchMessageStart, pchMessageStart, sizeof(pchMessageStart)) != 0) {
            if (fDebug10) printf("\n\nPROCESSMESSAGE: INVALID MESSAGESTART\n\n");
            fOk = false;
            break;
        }

        // Read header
        CMessageHeader& hdr = msg.hdr;
        if (!hdr.IsValid())
        {
            printf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand().c_str());
            continue;
        }
        string strCommand = hdr.GetCommand();


        // Message size
        unsigned int nMessageSize = hdr.nMessageSize;

		// Have a peek into what this node is doing
		if (false && LessVerbose(100))
		{
			std::string Peek = strCommand + ":" + RoundToString((double)nMessageSize,0) + " [" + NodeAddress(pfrom) + "]";
			AddPeek(Peek);
			std::string sCurrentCommand = RoundToString((double)GetAdjustedTime(),0) + Peek;
			std::string msLastNodeCommand = ReadCache("node_command",NodeAddress(pfrom));
			WriteCache("node_command",NodeAddress(pfrom),sCurrentCommand,GetAdjustedTime());
			if (msLastCommand == sCurrentCommand || (msLastNodeCommand == sCurrentCommand && !sCurrentCommand.empty()))
			{
   				  //Node Duplicates
				  double node_duplicates = cdbl(ReadCache("duplicates",NodeAddress(pfrom)),0) + 1;
				  WriteCache("duplicates",NodeAddress(pfrom),RoundToString(node_duplicates,0),GetAdjustedTime());
				  if ((node_duplicates > 350 && !OutOfSyncByAge()))
				  {
						printf(" Dupe (misbehaving) %s %s ",NodeAddress(pfrom).c_str(),Peek.c_str());
			     		pfrom->fDisconnect = true;
						WriteCache("duplicates",NodeAddress(pfrom),"0",GetAdjustedTime());
						return false;
				  }
     		}
			else
			{
				  double node_duplicates = cdbl(ReadCache("duplicates",NodeAddress(pfrom)),0) - 15;
				  if (node_duplicates < 1) node_duplicates = 0;
				  WriteCache("duplicates",NodeAddress(pfrom),RoundToString(node_duplicates,0),GetAdjustedTime());
			}
			msLastCommand = sCurrentCommand;
		}


        // Checksum
        CDataStream& vRecv = msg.vRecv;
        uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
        unsigned int nChecksum = 0;
        memcpy(&nChecksum, &hash, sizeof(nChecksum));
        if (nChecksum != hdr.nChecksum)
        {
            printf("ProcessMessages(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
               strCommand.c_str(), nMessageSize, nChecksum, hdr.nChecksum);
            continue;
        }

        // Process message
        bool fRet = false;
        try
        {
            {
                LOCK(cs_main);
                fRet = ProcessMessage(pfrom, strCommand, vRecv, msg.nTime);
            }
            if (fShutdown)
                break;
        }
        catch (std::ios_base::failure& e)
        {
            if (strstr(e.what(), "end of data"))
            {
                // Allow exceptions from under-length message on vRecv
                printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n", strCommand.c_str(), nMessageSize, e.what());
            }
            else if (strstr(e.what(), "size too large"))
            {
                // Allow exceptions from over-long size
                printf("ProcessMessages(%s, %u bytes) : Exception '%s' caught\n", strCommand.c_str(), nMessageSize, e.what());
            }
            else
            {
                PrintExceptionContinue(&e, "ProcessMessages()");
            }
        }
        catch (std::exception& e) {
            PrintExceptionContinue(&e, "ProcessMessages()");
        } catch (...) {
            PrintExceptionContinue(NULL, "ProcessMessages()");
        }

        if (!fRet)
		{
           if (fDebug10)   printf("ProcessMessage(%s, %u bytes) FAILED\n", strCommand.c_str(), nMessageSize);
		}
    }

    // In case the connection got shut down, its receive buffer was wiped
    if (!pfrom->fDisconnect)
        pfrom->vRecvMsg.erase(pfrom->vRecvMsg.begin(), it);

    return fOk;
}



double checksum(std::string s)
{
	char ch;
	double chk = 0;
	std::string sOut = "";
	for (unsigned int i=0;i < s.length(); i++)
	{
		ch = s.at(i);
		int ascii = ch;
		chk=chk+ascii;
	}
	return chk;
}




 uint256 GetScryptHashString(std::string s)
 {
        uint256 thash = 0;
		thash = Hash(BEGIN(s),END(s));
        return thash;
 }



uint256 GridcoinMultipleAlgoHash(std::string t1)
{
        uint256 thash = 0;
    	std::string& t2 = t1;
		thash = Hash(t2.begin(),t2.begin()+t2.length());
		return thash;
}


std::string aes_complex_hash(uint256 scrypt_hash)
{
	  if (scrypt_hash==0) return "0";

			std::string	sScryptHash = scrypt_hash.GetHex();
			std::string	sENCAES512 = AdvancedCrypt(sScryptHash);
			double chk = checksum(sENCAES512);
			uint256     hashSkein = GridcoinMultipleAlgoHash(sScryptHash);
			hashSkein = hashSkein + chk;
			std::string sSkeinAES512 = hashSkein.GetHex();
			return sSkeinAES512;
}


double LederstrumpfMagnitude_Retired(double Magnitude, int64_t locktime)
{
	// Note: This function returns an exponentially smaller result as the magnitude approaches the cap
	// The function is fine; keeping it in the coin for reference.  We moved to the new function when we thought the calculation was malfunctioning.  Later tests proved it works fine.
	// Establish constants
    double e = 2.718;
	double v = 4;
	double r = 0.915;
	double x = 0;
	double new_magnitude = 0;
	double user_magnitude_cap = 3000; //This is where the full 500 magnitude is achieved
	//Function returns a new magnitude between Magnitudes > 80% of Cap and <= Cap;
	double MagnitudeCap_LowSide = GetMaximumBoincSubsidy(locktime)*.80;
	if (Magnitude < MagnitudeCap_LowSide) return Magnitude;
	x = Magnitude / (user_magnitude_cap);
	new_magnitude = ((MagnitudeCap_LowSide/2) / (1 + pow((double)e, (double)(-v * (x - r))))) + MagnitudeCap_LowSide;
	//Debug.Print "With RAC of " + Trim(Rac) + ", NetRac of " + Trim(NetworkRac) + ", Subsidy = " + Trim(Subsidy)
	if (new_magnitude < MagnitudeCap_LowSide) new_magnitude=MagnitudeCap_LowSide;
	if (new_magnitude > GetMaximumBoincSubsidy(locktime)) new_magnitude=GetMaximumBoincSubsidy(locktime);
	return new_magnitude;
}


double LederstrumpfMagnitude2(double Magnitude, int64_t locktime)
{
	//2-1-2015 - Halford - The MagCap is 2000
	double MagCap = 2000;
	double out_mag = Magnitude;
	if (Magnitude >= MagCap*.90 && Magnitude <= MagCap*1.0) out_mag = MagCap*.90;
	if (Magnitude >= MagCap*1.0 && Magnitude <= MagCap*1.1) out_mag = MagCap*.91;
	if (Magnitude >= MagCap*1.1 && Magnitude <= MagCap*1.2) out_mag = MagCap*.92;
	if (Magnitude >= MagCap*1.2 && Magnitude <= MagCap*1.3) out_mag = MagCap*.93;
	if (Magnitude >= MagCap*1.3 && Magnitude <= MagCap*1.4) out_mag = MagCap*.94;
	if (Magnitude >= MagCap*1.4 && Magnitude <= MagCap*1.5) out_mag = MagCap*.95;
	if (Magnitude >= MagCap*1.5 && Magnitude <= MagCap*1.6) out_mag = MagCap*.96;
	if (Magnitude >= MagCap*1.6 && Magnitude <= MagCap*1.7) out_mag = MagCap*.97;
	if (Magnitude >= MagCap*1.7 && Magnitude <= MagCap*1.8) out_mag = MagCap*.98;
	if (Magnitude >= MagCap*1.8 && Magnitude <= MagCap*1.9) out_mag = MagCap*.99;
	if (Magnitude >= MagCap*1.9)						    out_mag = MagCap*1.0;
	return out_mag;
}



bool Contains(std::string data, std::string instring)
{
	std::size_t found = 0;
	found = data.find(instring);
	if (found != std::string::npos) return true;
	return false;
}


std::string NN(std::string value)
{
	return value.empty() ? "" : value;
}

double PendingSuperblockHeight()
{
	double height = cdbl(ReadCache("neuralsecurity","pending"),0);
	if (height < (double)(pindexBest->nHeight-200)) height = 0;
	return height;
}

std::string GetNeuralNetworkSuperBlock()
{
	//Only try to stake a superblock if the contract expired And the superblock is the highest popularity block And we do not have a pending superblock
	int64_t superblock_age = GetAdjustedTime() - mvApplicationCacheTimestamp["superblock;magnitudes"];
	if (NeuralNodeParticipates() && NeedASuperblock() && PendingSuperblockHeight()==0)
	{
		std::string myNeuralHash = "";
		#if defined(WIN32) && defined(QT_GUI)
	           myNeuralHash = qtGetNeuralHash("");
		#endif
 	    double popularity = 0;
		std::string consensus_hash = GetNeuralNetworkSupermajorityHash(popularity);
		if (fDebug2 && LessVerbose(5)) printf("SB Age %f, MyHash %s, ConsensusHash %s",(double)superblock_age,myNeuralHash.c_str(),consensus_hash.c_str());
		if (consensus_hash==myNeuralHash)
		{
			//Stake the contract
			std::string contract = "";
			#if defined(WIN32) && defined(QT_GUI)
				contract = qtGetNeuralContract("");
				if (fDebug2 && LessVerbose(5)) printf("Appending SuperBlock %f\r\n",(double)contract.length());
				if (AreBinarySuperblocksEnabled(nBestHeight))
				{
					// 12-21-2015 : Stake a binary superblock
					contract = PackBinarySuperblock(contract);
				}
			#endif
			return contract;
		}

	}
	return "";

}

std::string GetLastPORBlockHash(std::string cpid)
{
	StructCPID stCPID = GetInitializedStructCPID2(cpid,mvResearchAge);
	return stCPID.BlockHash;
}

std::string SerializeBoincBlock(MiningCPID mcpid)
{
	std::string delim = "<|>";
	std::string version = FormatFullVersion();
	mcpid.GRCAddress = DefaultWalletAddress();
	if (!IsResearchAgeEnabled(pindexBest->nHeight))
	{
		mcpid.Organization = DefaultOrg();
		mcpid.OrganizationKey = DefaultBlockKey(8); //Only reveal 8 characters
	}
	else
	{
		mcpid.projectname = "";
		mcpid.rac = 0;
		mcpid.NetworkRAC = 0;
	}

	std::string sNeuralHash = "";
	// To save network bandwidth, start posting the neural hashes in the CurrentNeuralHash field, so that out of sync neural network nodes can request neural data from those that are already synced and agree with the supermajority over the last 24 hrs
	if (!OutOfSyncByAge())
	{
		#if defined(WIN32) && defined(QT_GUI)
			sNeuralHash = qtGetNeuralHash("");
			mcpid.CurrentNeuralHash = sNeuralHash;
		#endif
	}

	//Add the neural hash only if necessary
	if (!OutOfSyncByAge() && NeuralNodeParticipates() && NeedASuperblock())
	{
		#if defined(WIN32) && defined(QT_GUI)
			mcpid.NeuralHash = sNeuralHash;
			mcpid.superblock = GetNeuralNetworkSuperBlock();
		#endif
	}

	mcpid.LastPORBlockHash = GetLastPORBlockHash(mcpid.cpid);

	if (mcpid.lastblockhash.empty()) mcpid.lastblockhash = "0";
	if (mcpid.LastPORBlockHash.empty()) mcpid.LastPORBlockHash="0";

	// If this is a POR, sign the block proving ownership of the CPID

	if (!mcpid.cpid.empty() && mcpid.cpid != "INVESTOR" && mcpid.lastblockhash != "0")
	{
		mcpid.BoincPublicKey = GetBeaconPublicKey(mcpid.cpid,false);
		if (!mcpid.BoincPublicKey.empty())
		{
			mcpid.BoincSignature = SignBlockWithCPID(mcpid.cpid,mcpid.lastblockhash);
			printf("\r\nSigning Block for cpid %s and blockhash %s with sig %s\r\n",mcpid.cpid.c_str(),mcpid.lastblockhash.c_str(),mcpid.BoincSignature.c_str());
		}
	}

	std::string bb = mcpid.cpid + delim + mcpid.projectname + delim + mcpid.aesskein + delim + RoundToString(mcpid.rac,0)
					+ delim + RoundToString(mcpid.pobdifficulty,5) + delim + RoundToString((double)mcpid.diffbytes,0)
					+ delim + NN(mcpid.enccpid)
					+ delim + NN(mcpid.encaes) + delim + RoundToString(mcpid.nonce,0) + delim + RoundToString(mcpid.NetworkRAC,0)
					+ delim + NN(version)
					+ delim + RoundToString(mcpid.ResearchSubsidy,2)
					+ delim + RoundToString(mcpid.LastPaymentTime,0)
					+ delim + RoundToString(mcpid.RSAWeight,0)
					+ delim + NN(mcpid.cpidv2)
					+ delim + RoundToString(mcpid.Magnitude,0)
					+ delim + NN(mcpid.GRCAddress) + delim + NN(mcpid.lastblockhash)
					+ delim + RoundToString(mcpid.InterestSubsidy,2) + delim + NN(mcpid.Organization)
					+ delim + NN(mcpid.OrganizationKey) + delim + mcpid.NeuralHash + delim + mcpid.superblock
					+ delim + RoundToString(mcpid.ResearchSubsidy2,2) + delim + RoundToString(mcpid.ResearchAge,6)
					+ delim + RoundToString(mcpid.ResearchMagnitudeUnit,6) + delim + RoundToString(mcpid.ResearchAverageMagnitude,2)
					+ delim + NN(mcpid.LastPORBlockHash) + delim + mcpid.CurrentNeuralHash + delim + mcpid.BoincPublicKey + delim + mcpid.BoincSignature;
	return bb;
}



MiningCPID DeserializeBoincBlock(std::string block)
{
	MiningCPID surrogate = GetMiningCPID();
	try
	{

	std::vector<std::string> s = split(block,"<|>");
	if (s.size() > 7)
	{
		surrogate.cpid = s[0];
		surrogate.projectname = s[1];
		boost::to_lower(surrogate.projectname);
      	surrogate.aesskein = s[2];
		surrogate.rac = cdbl(s[3],0);
		surrogate.pobdifficulty = cdbl(s[4],6);
		surrogate.diffbytes = (unsigned int)cdbl(s[5],0);
		surrogate.enccpid = s[6];
		surrogate.encboincpublickey = s[6];
		surrogate.encaes = s[7];
		surrogate.nonce = cdbl(s[8],0);
		if (s.size() > 9)
		{
			surrogate.NetworkRAC = cdbl(s[9],0);
		}
		if (s.size() > 10)
		{
			surrogate.clientversion = s[10];
		}
		if (s.size() > 11)
		{
			surrogate.ResearchSubsidy = cdbl(s[11],2);
		}
		if (s.size() > 12)
		{
			surrogate.LastPaymentTime = cdbl(s[12],0);
		}
		if (s.size() > 13)
		{
			surrogate.RSAWeight = cdbl(s[13],0);
		}
		if (s.size() > 14)
		{
			surrogate.cpidv2 = s[14];
		}
		if (s.size() > 15)
		{
			surrogate.Magnitude = cdbl(s[15],0);
		}
		if (s.size() > 16)
		{
			surrogate.GRCAddress = s[16];
		}
		if (s.size() > 17)
		{
			surrogate.lastblockhash = s[17];
		}
		if (s.size() > 18)
		{
			surrogate.InterestSubsidy = cdbl(s[18],2);
		}
		if (s.size() > 19)
		{
			surrogate.Organization = s[19];
		}
		if (s.size() > 20)
		{
			surrogate.OrganizationKey = s[20];
		}
		if (s.size() > 21)
		{
			surrogate.NeuralHash = s[21];
		}
		if (s.size() > 22)
		{
			surrogate.superblock = s[22];
		}
		if (s.size() > 23)
		{
			surrogate.ResearchSubsidy2 = cdbl(s[23],2);
		}
		if (s.size() > 24)
		{
			surrogate.ResearchAge = cdbl(s[24],6);
		}
		if (s.size() > 25)
		{
			surrogate.ResearchMagnitudeUnit = cdbl(s[25],6);
		}
		if (s.size() > 26)
		{
			surrogate.ResearchAverageMagnitude = cdbl(s[26],2);
		}
		if (s.size() > 27)
		{
			surrogate.LastPORBlockHash = s[27];
		}
		if (s.size() > 28)
		{
			surrogate.CurrentNeuralHash = s[28];
		}
		if (s.size() > 29)
		{
			surrogate.BoincPublicKey = s[29];
		}
		if (s.size() > 30)
		{
			surrogate.BoincSignature = s[30];
		}

	}
	}
	catch (...)
	{
		    printf("Deserialize ended with an error (06182014) \r\n");
	}
	return surrogate;
}



void printbool(std::string comment, bool boo)
{
	std::string b = boo ? "TRUE" : "FALSE";
	printf("%s : %s",comment.c_str(),b.c_str());
}



std::string ComputeCPIDv2(std::string email, std::string bpk, uint256 blockhash)
{
		//if (GetBoolArg("-disablecpidv2")) return "";
		CPID c = CPID();
		std::string cpid_non = bpk+email;
		std::string digest = c.CPID_V2(email,bpk,blockhash);
		return digest;
}



std::string boinc_hash(const std::string str)
{
	// Return the boinc hash of a string:
    CPID c = CPID(str);
    return c.hexdigest();
}




void InitializeProjectStruct(StructCPID& project)
{
	std::string email = GetArgument("email", "NA");
	boost::to_lower(email);

	project.email = email;
	std::string cpid_non = project.cpidhash+email;
	project.boincruntimepublickey = project.cpidhash;
	project.cpid = boinc_hash(cpid_non);
	std::string ENCbpk = AdvancedCrypt(cpid_non);
	project.boincpublickey = ENCbpk;
	project.cpidv2 = ComputeCPIDv2(email, project.cpidhash, 0);
	// (Old netsoft link) project.link = "http://boinc.netsoft-online.com/get_user.php?cpid=" + project.cpid;
	project.link = "http://boinc.netsoft-online.com/e107_plugins/boinc/get_user.php?cpid=" + project.cpid;
	//Local CPID with struct
	//Must contain cpidv2, cpid, boincpublickey
	project.Iscpidvalid = IsLocalCPIDValid(project);
 	if (fDebug10) printf("Memorizing local project %s, CPID Valid: %s;    ",project.projectname.c_str(),YesNo(project.Iscpidvalid).c_str());

}





std::string GetNetsoftProjects(std::string cpid)
{
			std::string cc = GetHttpPageFromCreditServerRetired(cpid,true,true);
			if (cc.length() < 10)
			{
				if (fDebug10) printf("Note: HTTP Page returned blank from netsoft for %s\r\n",cpid.c_str());
				return "";
			}

			int iRow = 0;
			std::vector<std::string> vCC = split(cc.c_str(),"<project>");

			if (vCC.size() > 1)
			{
				for (unsigned int i = 0; i < vCC.size(); i++)
				{
					std::string sProj  = ExtractXML(vCC[i],"<name>","</name>");
					std::string utc    = ExtractXML(vCC[i],"<total_credit>","</total_credit>");
					std::string rac    = ExtractXML(vCC[i],"<expavg_credit>","</expavg_credit>");
					std::string team   = ExtractXML(vCC[i],"<team_name>","</team_name>");
					std::string rectime= ExtractXML(vCC[i],"<expavg_time>","</expavg_time>");
					boost::to_lower(sProj);
					sProj = ToOfficialName(sProj);
					if (sProj == "mindmodeling@home") sProj = "mindmodeling@beta";
					if (sProj == "Quake Catcher Network") sProj = "Quake-Catcher Network";

					if (sProj.length() > 3)
					{
						std::string sKey = cpid + "+" + sProj;
						StructCPID strDPOR = GetInitializedStructCPID2(sKey,mvDPOR);
						iRow++;
						strDPOR.cpid = cpid;
						strDPOR.NetsoftRAC = cdbl(rac,0);
						mvDPOR[sKey] = strDPOR;
					}
				}
			}

			return cc;

}




void CreditCheckRetired(std::string cpid, bool clearcache)
{
	try {

			std::string cc = GetHttpPageFromCreditServerRetired(cpid,true,clearcache);
			if (cc.length() < 50)
			{
				if (fDebug10) printf("Note: HTTP Page returned blank from netsoft for %s\r\n",cpid.c_str());
				return;
			}

			int iRow = 0;
			std::vector<std::string> vCC = split(cc.c_str(),"<project>");
			if (vCC.size() > 0)
			{
				for (unsigned int i = 0; i < vCC.size(); i++)
				{
					std::string sProj  = ExtractXML(vCC[i],"<name>","</name>");
					std::string utc    = ExtractXML(vCC[i],"<total_credit>","</total_credit>");
					std::string rac    = ExtractXML(vCC[i],"<expavg_credit>","</expavg_credit>");
					std::string team   = ExtractXML(vCC[i],"<team_name>","</team_name>");
					std::string rectime= ExtractXML(vCC[i],"<expavg_time>","</expavg_time>");
					std::string proj_id= ExtractXML(vCC[i],"<project_id>","</project_id>");

					boost::to_lower(sProj);
					sProj = ToOfficialName(sProj);

					if (sProj.length() > 3 && !proj_id.empty())
					{
						StructCPID structcc = GetInitializedStructCPID2(sProj,mvCPIDs);
						iRow++;
						structcc.cpid = cpid;
						structcc.projectname = sProj;
						boost::to_lower(team);
						structcc.verifiedteam = team;
						if (structcc.verifiedteam != "gridcoin") structcc.rac = -1;
						structcc.verifiedrectime = cdbl(rectime,0);
						structcc.verifiedrac = cdbl(rac,0);
						structcc.rac = cdbl(rac,0);
						double currenttime =  GetAdjustedTime();
						double nActualTimespan = currenttime - structcc.verifiedrectime;
						structcc.verifiedage = nActualTimespan;
						mvCPIDs[sProj] = structcc;
						//////////////////////////// Store this information by CPID+Project also:
						std::string sKey = cpid + ":" + sProj;

					}
				}
			}

	}
	catch (std::exception &e)
	{
			 printf("error while accessing credit check online.\r\n");
	}
    catch(...)
	{
			printf("Error While accessing credit check online (2).\r\n");
	}


}





bool ProjectIsValid(std::string project)
{
	boost::to_lower(project);

	StructCPID structcpid = GetInitializedStructCPID2(project,mvBoincProjects);

	return structcpid.initialized;

}

std::string ToOfficialName(std::string proj)
{

			return ToOfficialNameNew(proj);

			/*
			boost::to_lower(proj);
			//Convert local XML project name [On the Left] to official [Netsoft] projectname:
			if (proj=="boincsimap")             proj = "simap";
			if (proj=="pogs")                   proj = "theskynet pogs";
			if (proj=="convector.fsv.cvut.cz")  proj = "convector";
			if (proj=="distributeddatamining")  proj = "distributed data mining";
			if (proj=="distrrtgen")             proj = "distributed rainbow table generator";
			if (proj=="eon2")                   proj = "eon";
			if (proj=="test4theory@home")       proj = "test4theory";
			if (proj=="lhc@home")               proj = "lhc@home 1.0";
			if (proj=="mindmodeling@beta")      proj = "mindmodeling@beta";
			if (proj=="volpex@uh")              proj = "volpex";
			if (proj=="oproject")               proj = "oproject@home";
			if (proj=="universe@home test")     proj = "universe@home";
			if (proj=="find@home")              proj = "fightmalaria";
			if (proj=="virtuallhc@home")        proj = "vLHCathome";
			return proj;
			*/
}

std::string strReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != std::string::npos){
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
  return str;
}

std::string LowerUnderscore(std::string data)
{
	boost::to_lower(data);
	data = strReplace(data,"_"," ");
	return data;
}

std::string ToOfficialNameNew(std::string proj)
{
	    proj = LowerUnderscore(proj);
		//Convert local XML project name [On the Left] to official [Netsoft] projectname:
		std::string sType = "projectmapping";
	    for(map<string,string>::iterator ii=mvApplicationCache.begin(); ii!=mvApplicationCache.end(); ++ii)
	    {
				std::string key_name  = (*ii).first;
			   	if (key_name.length() > sType.length())
				{
					if (key_name.substr(0,sType.length())==sType)
					{
							std::string key_value = mvApplicationCache[(*ii).first];
							std::vector<std::string> vKey = split(key_name,";");
							if (vKey.size() > 0)
							{
								std::string project_boinc   = vKey[1];
								std::string project_netsoft = key_value;
								proj=LowerUnderscore(proj);
								project_boinc=LowerUnderscore(project_boinc);
								project_netsoft=LowerUnderscore(project_netsoft);
								if (proj==project_boinc) proj=project_netsoft;
							}
					 }
				}
	    }
		return proj;
}




std::string GetBoincDataDir2()
{
	std::string path = "";
	/*       Default setting: boincdatadir=c:\\programdata\\boinc\\   */


    if (mapArgs.count("-boincdatadir"))
	{
        path = mapArgs["-boincdatadir"];
		if (path.length() > 0) return path;
    }

    #ifndef WIN32
    #ifdef __linux__
        path = "/var/lib/boinc-client/"; // Linux
    #else
        path = "/Library/Application Support/BOINC Data/"; // Mac OSX
    #endif
    #elif WINVER < 0x0600
        path = "c:\\documents and settings\\all users\\application data\\boinc\\"; // Windows XP
    #else
        path = "c:\\programdata\\boinc\\"; // Windows Vista and up
    #endif

    return path;
}


std::string GetArgument(std::string arg, std::string defaultvalue)
{
	std::string result = defaultvalue;
	if (mapArgs.count("-" + arg))
	{
		result = GetArg("-" + arg, defaultvalue);
	}
	return result;

}



void HarvestCPIDs(bool cleardata)
{

	if (fDebug10) printf("loading BOINC cpids ...\r\n");

	//Remote Boinc Feature - R Halford
	std::string sBoincKey = GetArgument("boinckey","");

	if (!sBoincKey.empty())
	{
		//Deserialize key into Global CPU Mining CPID 2-6-2015
		printf("Using key %s \r\n",sBoincKey.c_str());

		std::string sDec=DecodeBase64(sBoincKey);
		printf("Using key %s \r\n",sDec.c_str());

	    if (sDec.empty()) printf("Error while deserializing boinc key!  Please use execute genboinckey to generate a boinc key from the host with boinc installed.\r\n");
		GlobalCPUMiningCPID = DeserializeBoincBlock(sDec);

		GlobalCPUMiningCPID.initialized = true;

		if (GlobalCPUMiningCPID.cpid.empty())
		{
				 printf("Error while deserializing boinc key!  Please use execute genboinckey to generate a boinc key from the host with boinc installed.\r\n");
		}
		else
		{
			printf("CPUMiningCPID Initialized.\r\n");
		}

			GlobalCPUMiningCPID.email = GlobalCPUMiningCPID.aesskein;
			printf("Using Serialized Boinc CPID %s with orig email of %s and bpk of %s with cpidhash of %s \r\n",GlobalCPUMiningCPID.cpid.c_str(), GlobalCPUMiningCPID.email.c_str(), GlobalCPUMiningCPID.boincruntimepublickey.c_str(),GlobalCPUMiningCPID.cpidhash.c_str());
			GlobalCPUMiningCPID.cpidhash = GlobalCPUMiningCPID.boincruntimepublickey;
			printf("Using Serialized Boinc CPID %s with orig email of %s and bpk of %s with cpidhash of %s \r\n",GlobalCPUMiningCPID.cpid.c_str(), GlobalCPUMiningCPID.email.c_str(), GlobalCPUMiningCPID.boincruntimepublickey.c_str(),GlobalCPUMiningCPID.cpidhash.c_str());
			StructCPID structcpid = GetStructCPID();
			structcpid.initialized = true;
			structcpid.cpidhash = GlobalCPUMiningCPID.cpidhash;
			structcpid.projectname = GlobalCPUMiningCPID.projectname;
			structcpid.team = "gridcoin"; //Will be verified later during Netsoft Call
			structcpid.verifiedteam = "gridcoin";
			structcpid.rac = GlobalCPUMiningCPID.rac;
			structcpid.cpid = GlobalCPUMiningCPID.cpid;
			structcpid.boincpublickey = GlobalCPUMiningCPID.encboincpublickey;
			structcpid.boincruntimepublickey = structcpid.cpidhash;
			structcpid.NetworkRAC = GlobalCPUMiningCPID.NetworkRAC;
			structcpid.email = GlobalCPUMiningCPID.email;
			// 2-6-2015 R Halford - Ensure CPIDv2 Is populated After deserializing GenBoincKey
			std::string cpid_non = structcpid.cpidhash+structcpid.email;
			printf("GenBoincKey using email %s and cpidhash %s key %s \r\n",structcpid.email.c_str(),structcpid.cpidhash.c_str(),sDec.c_str());
			structcpid.cpidv2 = ComputeCPIDv2(structcpid.email, structcpid.cpidhash, 0);
			// Old link: structcpid.link = "http://boinc.netsoft-online.com/get_user.php?cpid=" + structcpid.cpid;
			structcpid.link = "http://boinc.netsoft-online.com/e107_plugins/boinc/get_user.php?cpid=" + structcpid.cpid;
			structcpid.Iscpidvalid = true;
			mvCPIDs.insert(map<string,StructCPID>::value_type(structcpid.projectname,structcpid));
			// CreditCheck(structcpid.cpid,false);
			GetNextProject(false);
			if (fDebug10) printf("GCMCPI %s",GlobalCPUMiningCPID.cpid.c_str());
			if (fDebug10) 			printf("Finished getting first remote boinc project\r\n");
		return;
  }

 try
 {

	std::string sourcefile = GetBoincDataDir2() + "client_state.xml";
    std::string sout = "";
    sout = getfilecontents(sourcefile);
	if (sout == "-1")
	{
		printf("Unable to obtain Boinc CPIDs \r\n");

		if (mapArgs.count("-boincdatadir") && mapArgs["-boincdatadir"].length() > 0)
		{
			printf("Boinc data directory set in gridcoinresearch.conf has been incorrectly specified \r\n");
		}

		else printf("Boinc data directory is not in the operating system's default location \r\nPlease move it there or specify its current location in gridcoinresearch.conf \r\n");

		return;
	}

	if (cleardata)
	{
		mvCPIDs.clear();
		mvCPIDCache.clear();
	}
	std::string email = GetArgument("email","");
    boost::to_lower(email);

	int iRow = 0;
	std::vector<std::string> vCPID = split(sout.c_str(),"<project>");
	std::string investor = GetArgument("investor","false");

	if (investor=="true")
	{
			msPrimaryCPID="INVESTOR";
	}
	else
	{

			if (vCPID.size() > 0)
			{
				for (unsigned int i = 0; i < vCPID.size(); i++)
				{
					std::string email_hash = ExtractXML(vCPID[i],"<email_hash>","</email_hash>");
					std::string cpidhash = ExtractXML(vCPID[i],"<cross_project_id>","</cross_project_id>");
					std::string externalcpid = ExtractXML(vCPID[i],"<external_cpid>","</external_cpid>");
					std::string utc=ExtractXML(vCPID[i],"<user_total_credit>","</user_total_credit>");
					std::string rac=ExtractXML(vCPID[i],"<user_expavg_credit>","</user_expavg_credit>");
					std::string proj=ExtractXML(vCPID[i],"<project_name>","</project_name>");
					std::string team=ExtractXML(vCPID[i],"<team_name>","</team_name>");
					std::string rectime = ExtractXML(vCPID[i],"<rec_time>","</rec_time>");

					boost::to_lower(proj);
					proj = ToOfficialName(proj);
					ProjectIsValid(proj);
					int64_t nStart = GetTimeMillis();
					if (cpidhash.length() > 5 && proj.length() > 3)
					{
						std::string cpid_non = cpidhash+email;
						to_lower(cpid_non);
						StructCPID structcpid = GetInitializedStructCPID2(proj,mvCPIDs);
						iRow++;
						structcpid.cpidhash = cpidhash;
						structcpid.projectname = proj;
						boost::to_lower(team);
						structcpid.team = team;
						InitializeProjectStruct(structcpid);
						int64_t elapsed = GetTimeMillis()-nStart;
						if (fDebug3) printf("Enumerating boinc local project %s cpid %s valid %s, elapsed %f ",structcpid.projectname.c_str(),structcpid.cpid.c_str(),YesNo(structcpid.Iscpidvalid).c_str(),(double)elapsed);
						structcpid.rac = cdbl(rac,0);
						structcpid.verifiedrac = cdbl(rac,0);
						std::string sLocalClientEmailHash = RetrieveMd5(email);

						if (email_hash != sLocalClientEmailHash)
						{
							structcpid.errors = "Gridcoin Email setting does not match project Email.  Check Gridcoin e-mail address setting or boinc project e-mail setting.";
							structcpid.Iscpidvalid=false;
						}


						if (!structcpid.Iscpidvalid)
						{
							structcpid.errors = "CPID calculation invalid.  Check e-mail address and try resetting the boinc project.";
						}

						structcpid.utc = cdbl(utc,0);
						structcpid.rectime = cdbl(rectime,0);
						double currenttime =  GetAdjustedTime();
						double nActualTimespan = currenttime - structcpid.rectime;
						structcpid.age = nActualTimespan;
						std::string sKey = structcpid.cpid + ":" + proj;
						mvCPIDs[proj] = structcpid;
	         	
						if (!structcpid.Iscpidvalid)
						{
							structcpid.errors = "CPID invalid.  Check E-mail address.";
						}
			
						if (structcpid.team != "gridcoin")
						{
							structcpid.Iscpidvalid = false;
							structcpid.errors = "Team invalid";
						}
						bool bTestExternal = true;
						bool bTestInternal = true;

						if (!externalcpid.empty())
						{
							printf("\r\n** External CPID not empty %s **\r\n",externalcpid.c_str());

							bTestExternal = CPIDAcidTest2(cpidhash,externalcpid);
							bTestInternal = CPIDAcidTest2(cpidhash,structcpid.cpid);
							if (bTestExternal)
							{
								structcpid.cpid = externalcpid;
								printf(" Setting CPID to %s ",structcpid.cpid.c_str());
							}
							else
							{
								printf("External test failed.");
							}


							if (!bTestExternal && !bTestInternal)
							{
								structcpid.Iscpidvalid = false;
								structcpid.errors = "CPID corrupted Internal: %s, External: %s" + structcpid.cpid + "," + externalcpid.c_str();
								printf("CPID corrupted Internal: %s, External: %s \r\n",structcpid.cpid.c_str(),externalcpid.c_str());
							}
							mvCPIDs[proj] = structcpid;
					    }

						if (structcpid.Iscpidvalid)
						{
								// Verify the CPID has magnitude > 0, otherwise set the user as an investor:
								int iCPIDType = DetermineCPIDType(structcpid.cpid);
								// -1 = Invalid CPID
								//  1 = Valid CPID with RAC
								//  2 = Investor or Pool Miner
								if (iCPIDType==1)
								{
									GlobalCPUMiningCPID.cpidhash = cpidhash;
									GlobalCPUMiningCPID.email = email;
									GlobalCPUMiningCPID.boincruntimepublickey = cpidhash;
									printf("\r\nSetting bpk to %s\r\n",cpidhash.c_str());

									if (structcpid.team=="gridcoin")
									{
										msPrimaryCPID = structcpid.cpid;
										#if defined(WIN32) && defined(QT_GUI)
											//Let the Neural Network know what your CPID is so it can be charted:
											std::string sXML = "<KEY>PrimaryCPID</KEY><VALUE>" + msPrimaryCPID + "</VALUE>";
											std::string sData = qtExecuteDotNetStringFunction("WriteKey",sXML);
										#endif
										//Try to get a neural RAC report
										AsyncNeuralRequest("explainmag",msPrimaryCPID,5);
									}
								}
						}

						mvCPIDs[proj] = structcpid;
						if (fDebug10) printf("Adding Local Project %s \r\n",structcpid.cpid.c_str());

					}

				}

			}
			// If no valid boinc projects were found:
			if (msPrimaryCPID.empty()) msPrimaryCPID="INVESTOR";

		}
	}
	catch (std::exception &e)
	{
			 printf("Error while harvesting CPIDs.\r\n");
	}
    catch(...)
	{
		     printf("Error while harvesting CPIDs 2.\r\n");
	}



}



void ThreadCPIDs()
{
	RenameThread("grc-cpids");
    bCPIDsLoaded = false;
	HarvestCPIDs(true);
	bCPIDsLoaded = true;
	//CreditCheck(GlobalCPUMiningCPID.cpid,false);
	printf("Getting first project");
	GetNextProject(false);
	printf("Finished getting first project");
	bProjectsInitialized = true;
}


void LoadCPIDsInBackground()
{
	  if (IsLockTimeWithinMinutes(nCPIDsLoaded,10)) return;
	  nCPIDsLoaded = GetAdjustedTime();
	  cpidThreads = new boost::thread_group();
	  cpidThreads->create_thread(boost::bind(&ThreadCPIDs));
}

StructCPID GetStructCPID()
{
	StructCPID c;
	c.initialized=false;
	c.isvoucher=false;
	c.rac = 0;
	c.utc=0;
	c.rectime=0;
	c.age = 0;
	c.activeproject=false;
	c.verifiedutc=0;
	c.verifiedrectime=0;
	c.verifiedage=0;
	c.entries=0;
	c.AverageRAC=0;
	c.NetworkProjects=0;
	c.Iscpidvalid=false;
	c.NetworkRAC=0;
	c.TotalRAC=0;
	c.TotalNetworkRAC=0;
	c.Magnitude=0;
	c.LastMagnitude=0;
	c.PaymentMagnitude=0;
	c.owed=0;
	c.payments=0;
	c.outstanding=0;
	c.verifiedTotalRAC=0;
	c.verifiedTotalNetworkRAC=0;
	c.verifiedMagnitude=0;
	c.TotalMagnitude=0;
	c.MagnitudeCount=0;
	c.LowLockTime=0;
	c.HighLockTime=0;
	c.Accuracy=0;
	c.totalowed=0;
	c.longtermtotalowed=0;
	c.longtermowed=0;
	c.LastPaymentTime=0;
	c.EarliestPaymentTime=0;
	c.RSAWeight=0;
	c.PaymentTimespan=0;
	c.ResearchSubsidy = 0;
	c.InterestSubsidy = 0;
	c.BTCQuote = 0;
	c.GRCQuote = 0;
	c.ResearchSubsidy2 = 0;
	c.ResearchAge = 0;
	c.ResearchMagnitudeUnit = 0;
	c.ResearchAverageMagnitude = 0;
	c.Canary = 0;
	c.NetsoftRAC = 0;
	c.interestPayments = 0;
	c.payments = 0;
	c.LastBlock = 0;
	c.NetworkMagnitude=0;
	c.NetworkAvgMagnitude=0;

	return c;

}

MiningCPID GetMiningCPID()
{
	MiningCPID mc;
	mc.rac = 0;
	mc.pobdifficulty = 0;
	mc.diffbytes = 0;
	mc.initialized = false;
	mc.nonce = 0;
	mc.NetworkRAC=0;
	mc.prevBlockType = 0;
	mc.lastblockhash = "0";
	mc.VouchedRAC = 0;
	mc.VouchedNetworkRAC  = 0;
	mc.Magnitude = 0;
	mc.Accuracy = 0;
	mc.RSAWeight = 0;
	mc.LastPaymentTime=0;
	mc.ResearchSubsidy = 0;
	mc.InterestSubsidy = 0;
	mc.GRCQuote = 0;
	mc.BTCQuote = 0;
	mc.ResearchSubsidy2 = 0;
	mc.ResearchAge = 0;
	mc.ResearchMagnitudeUnit = 0;
	mc.ResearchAverageMagnitude = 0;
	mc.Canary = 0; //Used to test for a memory overflow
	return mc;
}


void TrackRequests(CNode* pfrom,std::string sRequestType)
{
	    std::string sKey = "request_type" + sRequestType;
	    double dReqCt = cdbl(ReadCache(sKey,NodeAddress(pfrom)),0) + 1;
	    WriteCache(sKey,NodeAddress(pfrom),RoundToString(dReqCt,0),GetAdjustedTime());
        if ( (dReqCt > 20 && !OutOfSyncByAge()) )
		{
					printf(" Node requests for %s exceeded threshold (misbehaving) %s ",sRequestType.c_str(),NodeAddress(pfrom).c_str());
			        //pfrom->Misbehaving(1);
          			pfrom->fDisconnect = true;
					WriteCache(sKey,NodeAddress(pfrom),"0",GetAdjustedTime());
		}
}


bool SendMessages(CNode* pto, bool fSendTrickle)
{
    TRY_LOCK(cs_main, lockMain);
    if (lockMain) {
        // Don't send anything until we get their version message
        if (pto->nVersion == 0)
            return true;

        //
        // Message: ping
        //
        bool pingSend = false;
        if (pto->fPingQueued)
		{
            // RPC ping request by user
            pingSend = true;
        }
        if (pto->nPingNonceSent == 0 && pto->nPingUsecStart + PING_INTERVAL * 1000000 < GetTimeMicros())
		{
            // Ping automatically sent as a latency probe & keepalive.
            pingSend = true;
        }
        if (pingSend)
		{
		    uint64_t nonce = 0;
            while (nonce == 0) {
                RAND_bytes((unsigned char*)&nonce, sizeof(nonce));
            }
            pto->fPingQueued = false;
            pto->nPingUsecStart = GetTimeMicros();
            if (pto->nVersion > BIP0031_VERSION)
			{
                pto->nPingNonceSent = nonce;
				std::string acid = GetCommandNonce("ping");
                pto->PushMessage("ping", nonce, acid);
            } else
			{
                // Peer is too old to support ping command with nonce, pong will never arrive.
                pto->nPingNonceSent = 0;
                pto->PushMessage("ping");
            }
        }

        // Resend wallet transactions that haven't gotten in a block yet
	    ResendWalletTransactions();

        // Address refresh broadcast
        static int64_t nLastRebroadcast;
        if (!IsInitialBlockDownload() && ( GetAdjustedTime() - nLastRebroadcast > 24 * 60 * 60))
        {
            {
                LOCK(cs_vNodes);
                BOOST_FOREACH(CNode* pnode, vNodes)
                {
                    // Periodically clear setAddrKnown to allow refresh broadcasts
                    if (nLastRebroadcast)
                        pnode->setAddrKnown.clear();

                    // Rebroadcast our address
                    if (!fNoListen)
                    {
                        CAddress addr = GetLocalAddress(&pnode->addr);
                        if (addr.IsRoutable())
                            pnode->PushAddress(addr);
                    }
                }
            }
            nLastRebroadcast =  GetAdjustedTime();
        }

        //
        // Message: addr
        //
        if (fSendTrickle)
        {
            vector<CAddress> vAddr;
            vAddr.reserve(pto->vAddrToSend.size());
            BOOST_FOREACH(const CAddress& addr, pto->vAddrToSend)
            {
                // returns true if wasn't already contained in the set
                if (pto->setAddrKnown.insert(addr).second)
                {
                    vAddr.push_back(addr);
                    // receiver rejects addr messages larger than 1000
                    if (vAddr.size() >= 1000)
                    {
                        pto->PushMessage("gridaddr", vAddr);
                        vAddr.clear();
                    }
                }
            }
            pto->vAddrToSend.clear();
            if (!vAddr.empty())
                pto->PushMessage("gridaddr", vAddr);
        }


        //
        // Message: inventory
        //
        vector<CInv> vInv;
        vector<CInv> vInvWait;
        {
            LOCK(pto->cs_inventory);
            vInv.reserve(pto->vInventoryToSend.size());
            vInvWait.reserve(pto->vInventoryToSend.size());
            BOOST_FOREACH(const CInv& inv, pto->vInventoryToSend)
            {
                if (pto->setInventoryKnown.count(inv))
                    continue;

                // trickle out tx inv to protect privacy
                if (inv.type == MSG_TX && !fSendTrickle)
                {
                    // 1/4 of tx invs blast to all immediately
                    static uint256 hashSalt;
                    if (hashSalt == 0)
                        hashSalt = GetRandHash();
                    uint256 hashRand = inv.hash ^ hashSalt;
                    hashRand = Hash(BEGIN(hashRand), END(hashRand));
                    bool fTrickleWait = ((hashRand & 3) != 0);

                    // always trickle our own transactions
                    if (!fTrickleWait)
                    {
                        CWalletTx wtx;
                        if (GetTransaction(inv.hash, wtx))
                            if (wtx.fFromMe)
                                fTrickleWait = true;
                    }

                    if (fTrickleWait)
                    {
				        vInvWait.push_back(inv);
                        continue;
                    }
                }

			    // returns true if wasn't already contained in the set
                if (pto->setInventoryKnown.insert(inv).second)
                {
				     vInv.push_back(inv);
                     if (vInv.size() >= 1000)
                     {
						    if (false)
							{
								AddPeek("PushInv-Large " + RoundToString((double)vInv.size(),0));
								// If node has not been misbehaving (1-30-2016) then push it: (pto->nMisbehavior) && pto->NodeAddress().->addr.IsRoutable()
								pto->PushMessage("inv", vInv);
								AddPeek("Pushed Inv-Large " + RoundToString((double)vInv.size(),0));
								if (fDebug10) printf(" *PIL* ");
								vInv.clear();
								if (TimerMain("PushInventoryLarge",50)) CleanInboundConnections(true);
								// Eventually ban the node if they keep asking for inventory
								TrackRequests(pto,"Inv-Large");
								AddPeek("Done with Inv-Large " + RoundToString((double)vInv.size(),0));
							}
							else
							{
							    pto->PushMessage("inv", vInv);
								vInv.clear();
							}
       
			        }
                }
            }
            pto->vInventoryToSend = vInvWait;
        }
        if (!vInv.empty())
            pto->PushMessage("inv", vInv);


        //
        // Message: getdata
        //
        vector<CInv> vGetData;
        int64_t nNow =  GetAdjustedTime() * 1000000;
        CTxDB txdb("r");
        while (!pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow)
        {
            const CInv& inv = (*pto->mapAskFor.begin()).second;
            if (!AlreadyHave(txdb, inv))
            {
                if (fDebugNet)        printf("sending getdata: %s\n", inv.ToString().c_str());
				//AddPeek("Getdata " + inv.ToString());
                vGetData.push_back(inv);
                if (vGetData.size() >= 1000)
                {
                    pto->PushMessage("getdata", vGetData);
                    vGetData.clear();
                }
                mapAlreadyAskedFor[inv] = nNow;
            }
            pto->mapAskFor.erase(pto->mapAskFor.begin());
        }
        if (!vGetData.empty())
		{
            pto->PushMessage("getdata", vGetData);
			//AddPeek("GetData");
		}

    }
    return true;
}



std::string ReadCache(std::string section, std::string key)
{
	if (section.empty() || key.empty()) return "";

	try
	{
			std::string value = mvApplicationCache[section + ";" + key];
			if (value.empty())
			{
				mvApplicationCache.insert(map<std::string,std::string>::value_type(section + ";" + key,""));
				mvApplicationCache[section + ";" + key]="";
				return "";
			}
			return value;
	}
	catch(...)
	{
		printf("readcache error %s",section.c_str());
		return "";
	}
}


void WriteCache(std::string section, std::string key, std::string value, int64_t locktime)
{
	if (section.empty() || key.empty()) return;
	std::string temp_value = mvApplicationCache[section + ";" + key];
	if (temp_value.empty())
	{
		mvApplicationCache.insert(map<std::string,std::string>::value_type(section + ";" + key,value));
	    mvApplicationCache[section + ";" + key]=value;
	}
	mvApplicationCache[section + ";" + key]=value;
	// Record Cache Entry timestamp
	int64_t temp_locktime = mvApplicationCacheTimestamp[section + ";" + key];
	if (temp_locktime == 0)
	{
		mvApplicationCacheTimestamp.insert(map<std::string,int64_t>::value_type(section+";"+key,1));
		mvApplicationCacheTimestamp[section+";"+key]=locktime;
	}
	mvApplicationCacheTimestamp[section+";"+key] = locktime;

}



void PurgeCacheAsOfExpiration(std::string section, int64_t expiration)
{
	   for(map<string,string>::iterator ii=mvApplicationCache.begin(); ii!=mvApplicationCache.end(); ++ii)
	   {
				std::string key_section = mvApplicationCache[(*ii).first];
				if (key_section.length() > section.length())
				{
					if (key_section.substr(0,section.length())==section)
					{
						if (mvApplicationCacheTimestamp[key_section] < expiration)
						{
							printf("purging %s",key_section.c_str());
							mvApplicationCache[key_section]="";
							mvApplicationCacheTimestamp[key_section]=1;
						}
					}
				}
	   }

}


void ClearCache(std::string section)
{
	   for(map<string,string>::iterator ii=mvApplicationCache.begin(); ii!=mvApplicationCache.end(); ++ii)
	   {
				std::string key_section = mvApplicationCache[(*ii).first];
				if (key_section.length() > section.length())
				{
					if (key_section.substr(0,section.length())==section)
					{
						printf("\r\nClearing the cache....of value %s \r\n",mvApplicationCache[key_section].c_str());
						mvApplicationCache[key_section]="";
						mvApplicationCacheTimestamp[key_section]=1;
					}
				}
	   }

}


void DeleteCache(std::string section, std::string keyname)
{
	   std::string pk = section + ";" +keyname;
       mvApplicationCache.erase(pk);
	   mvApplicationCacheTimestamp.erase(pk);
}



void IncrementCurrentNeuralNetworkSupermajority(std::string NeuralHash, std::string GRCAddress, double distance)
{
	if (NeuralHash.length() < 5) return;
	double temp_hashcount = 0;
	if (mvCurrentNeuralNetworkHash.size() > 0)
	{
			temp_hashcount = mvCurrentNeuralNetworkHash[NeuralHash];
	}
	// 6-13-2015 ONLY Count Each Neural Hash Once per GRC address / CPID (1 VOTE PER RESEARCHER)
	std::string Security = ReadCache("currentneuralsecurity",GRCAddress);
	if (Security == NeuralHash)
	{
		//This node has already voted, throw away the vote
		return;
	}
	WriteCache("currentneuralsecurity",GRCAddress,NeuralHash,GetAdjustedTime());
	if (temp_hashcount == 0)
	{
		mvCurrentNeuralNetworkHash.insert(map<std::string,double>::value_type(NeuralHash,0));
	}
	double multiplier = 200;
	if (distance < 40) multiplier = 400;
	double votes = (1/distance)*multiplier;
	temp_hashcount += votes;
	mvCurrentNeuralNetworkHash[NeuralHash] = temp_hashcount;
}



void IncrementNeuralNetworkSupermajority(std::string NeuralHash, std::string GRCAddress, double distance)
{
	if (NeuralHash.length() < 5) return;
	double temp_hashcount = 0;
	if (mvNeuralNetworkHash.size() > 0)
	{
			temp_hashcount = mvNeuralNetworkHash[NeuralHash];
	}
	// 6-13-2015 ONLY Count Each Neural Hash Once per GRC address / CPID (1 VOTE PER RESEARCHER)
	std::string Security = ReadCache("neuralsecurity",GRCAddress);
	if (Security == NeuralHash)
	{
		//This node has already voted, throw away the vote
		return;
	}
	WriteCache("neuralsecurity",GRCAddress,NeuralHash,GetAdjustedTime());
	if (temp_hashcount == 0)
	{
		mvNeuralNetworkHash.insert(map<std::string,double>::value_type(NeuralHash,0));
	}
	double multiplier = 200;
	if (distance < 40) multiplier = 400;
	double votes = (1/distance)*multiplier;
	temp_hashcount += votes;
	mvNeuralNetworkHash[NeuralHash] = temp_hashcount;
}


void IncrementVersionCount(std::string Version)
{
	if (Version.empty()) return;
	double temp_vercount = 0;
	if (mvNeuralVersion.size() > 0)
	{
			temp_vercount = mvNeuralVersion[Version];
	}
	if (temp_vercount == 0)
	{
		mvNeuralVersion.insert(map<std::string,double>::value_type(Version,0));
	}
	temp_vercount += 1;
	mvNeuralVersion[Version] = temp_vercount;
}



std::string GetNeuralNetworkSupermajorityHash(double& out_popularity)
{
	double highest_popularity = -1;
	std::string neural_hash = "";
	for(map<std::string,double>::iterator ii=mvNeuralNetworkHash.begin(); ii!=mvNeuralNetworkHash.end(); ++ii)
	{
				double popularity = mvNeuralNetworkHash[(*ii).first];
				// d41d8 is the hash of an empty magnitude contract - don't count it
				if ( ((*ii).first != "d41d8cd98f00b204e9800998ecf8427e") && popularity > 0 && popularity > highest_popularity && (*ii).first != "TOTAL_VOTES")
				{
					highest_popularity = popularity;
					neural_hash = (*ii).first;
				}
	}
	out_popularity = highest_popularity;
	return neural_hash;
}


std::string GetCurrentNeuralNetworkSupermajorityHash(double& out_popularity)
{
	double highest_popularity = -1;
	std::string neural_hash = "";
	for(map<std::string,double>::iterator ii=mvCurrentNeuralNetworkHash.begin(); ii!=mvCurrentNeuralNetworkHash.end(); ++ii)
	{
				double popularity = mvCurrentNeuralNetworkHash[(*ii).first];
				// d41d8 is the hash of an empty magnitude contract - don't count it
				if ( ((*ii).first != "d41d8cd98f00b204e9800998ecf8427e") && popularity > 0 && popularity > highest_popularity && (*ii).first != "TOTAL_VOTES")
				{
					highest_popularity = popularity;
					neural_hash = (*ii).first;
				}
	}
	out_popularity = highest_popularity;
	return neural_hash;
}






std::string GetNeuralNetworkReport()
{
	//Returns a report of the networks neural hashes in order of popularity
	std::string neural_hash = "";
	std::string report = "Neural_hash, Popularity\r\n";
	std::string row = "";
	for(map<std::string,double>::iterator ii=mvNeuralNetworkHash.begin(); ii!=mvNeuralNetworkHash.end(); ++ii)
	{
				double popularity = mvNeuralNetworkHash[(*ii).first];
				neural_hash = (*ii).first;
				row = neural_hash+ "," + RoundToString(popularity,0);
				report += row + "\r\n";
	}

	return report;
}

std::string GetOrgSymbolFromFeedKey(std::string feedkey)
{
	std::string Symbol = ExtractValue(feedkey,"-",0);
	return Symbol;

}



bool MemorizeMessage(std::string msg, int64_t nTime, double dAmount, std::string sRecipient)
{
	      if (msg.empty()) return false;
		  bool fMessageLoaded = false;

		  if (Contains(msg,"<MT>"))
	 	  {
			  std::string sMessageType      = ExtractXML(msg,"<MT>","</MT>");
  			  std::string sMessageKey       = ExtractXML(msg,"<MK>","</MK>");
			  std::string sMessageValue     = ExtractXML(msg,"<MV>","</MV>");
			  std::string sMessageAction    = ExtractXML(msg,"<MA>","</MA>");
			  std::string sSignature        = ExtractXML(msg,"<MS>","</MS>");
			  std::string sMessagePublicKey = ExtractXML(msg,"<MPK>","</MPK>");
			  if (sMessageType=="beacon" && Contains(sMessageValue,"INVESTOR"))
			  {
					sMessageValue="";
			  }

			  if (sMessageType=="beacon" && sMessageAction=="A")
			  {
				    // If the Beacon Public Key is Not Empty - do not overwrite with a new beacon value unless the public key is the same
				    std::string sBPK = GetBeaconPublicKey(sMessageKey,false);
					if (!sBPK.empty())
					{
 						std::string out_cpid = "";
						std::string out_address = "";
						std::string out_publickey = "";
  						GetBeaconElements(sMessageValue, out_cpid, out_address, out_publickey);
				 	    if (fDebug3 && LessVerbose(50)) 
						{
					        printf("\r\n**Beacon Debug Message : beaconpubkey %s, message key %s, cpid %s, addr %s, base64 pub key %s \r\n ",sBPK.c_str(),
							  sMessageKey.c_str(),out_cpid.c_str(),out_address.c_str(), out_publickey.c_str());
						}
					    if (sBPK == out_publickey)
						{
							// allow key to be reloaded in since this is a refreshed beacon
							if (fDebug10) printf("\r\n**Beacon Being Overwritten %s \r\n %s : %s\r\n",sBPK.c_str(),sMessageKey.c_str(),sBPK.c_str());
						}
						else
						{
							// In this case, the current Beacon is not empty and the keys are different - Do not overwrite this beacon
							sMessageValue="";
							if (fDebug10) printf("\r\n**Beacon Public Key Not Empty %s : %s\r\n",sMessageKey.c_str(),sBPK.c_str());
						}
					}

			  }

			  if (!sMessageType.empty() && !sMessageKey.empty() && !sMessageValue.empty() && !sMessageAction.empty() && !sSignature.empty())
			  {

				  // If this is a DAO, ensure the contents are protected:
				  if ((sMessageType=="dao" || sMessageType=="daoclient") && !sMessagePublicKey.empty())
				  {
					        if (fDebug10) printf("DAO Message %s",msg.c_str());

							if (sMessageAction=="A")
							{
								std::string daoPubKey = ReadCache(sMessageType + "pubkey",sMessageKey);
								if (daoPubKey.empty())
								{
									//We only accept the first message
									WriteCache(sMessageType + "pubkey",sMessageKey,sMessagePublicKey,nTime);
									std::string OrgSymbol = ExtractXML(sMessageValue,"<SYMBOL>","</SYMBOL>");
									std::string OrgName = ExtractXML(sMessageValue,"<NAME>","</NAME>");
									std::string OrgREST = ExtractXML(sMessageValue,"<REST>","</REST>");
									WriteCache(sMessageType + "rest",  OrgSymbol,  OrgREST,    nTime);
									WriteCache(sMessageType + "symbol",sMessageKey,OrgSymbol,  nTime);
									WriteCache(sMessageType + "name",  OrgSymbol,  sMessageKey,nTime);
									WriteCache(sMessageType + "orgname", OrgSymbol,OrgName,    nTime);
								}
							}
				  }

				  if (sMessageType=="dao" || sMessageType=="daoclient")
				  {
						sMessagePublicKey = ReadCache(sMessageType+"pubkey",sMessageKey);
				  }
				  if (sMessageType == "daofeed")
				  {
			  			sMessagePublicKey = ReadCache("daopubkey",GetOrgSymbolFromFeedKey(sMessageKey));
				  }

				  //Verify sig first
				  bool Verified = CheckMessageSignature(sMessageAction,sMessageType,sMessageType+sMessageKey+sMessageValue,
					  sSignature,sMessagePublicKey);

				  if ( (sMessageType=="dao" || sMessageType == "daofeed") && !Verified && fDebug3)
				  {
						printf("Message type %s: %s was not verified successfully. PubKey %s \r\n",sMessageType.c_str(),msg.c_str(),sMessagePublicKey.c_str());
				  }

				  if (Verified)
				  {

						if (sMessageAction=="A")
						{
								if ( (sMessageType=="dao" || sMessageType == "daofeed")	&& fDebug3 )
									printf("Adding MessageKey type %s Key %s Value %s\r\n",
									sMessageType.c_str(),sMessageKey.c_str(),sMessageValue.c_str());
								// Ensure we have the TXID of the contract in memory
								if (!(sMessageType=="project" || sMessageType=="projectmapping" || sMessageType=="beacon" ))
								{
									WriteCache(sMessageType,sMessageKey+";Recipient",sRecipient,nTime);
									WriteCache(sMessageType,sMessageKey+";BurnAmount",RoundToString(dAmount,2),nTime);
								}
								WriteCache(sMessageType,sMessageKey,sMessageValue,nTime);
								fMessageLoaded = true;
								if (sMessageType=="poll")
								{
									    if (Contains(sMessageKey,"[Foundation"))
										{
												msMiningErrors2 = "Foundation Poll: " + sMessageKey;

										}
										else
										{
						    					msMiningErrors2 = "Poll: " + sMessageKey;
										}
								}

						}
						else if(sMessageAction=="D")
						{
								if (fDebug10) printf("Deleting key type %s Key %s Value %s\r\n",sMessageType.c_str(),sMessageKey.c_str(),sMessageValue.c_str());
								DeleteCache(sMessageType,sMessageKey);
								fMessageLoaded = true;
						}
						// If this is a boinc project, load the projects into the coin:
						if (sMessageType=="project" || sMessageType=="projectmapping")
						{
							//Reserved
							fMessageLoaded = true;
						}

				  }

				}

	}
   return fMessageLoaded;
}







bool UnusualActivityReport()
{

    map<uint256, CTxIndex> mapQueuedChanges;
    CTxDB txdb("r");
	int nMaxDepth = nBestHeight;
    CBlock block;
	int nMinDepth = fTestNet ? 1 : 1;
	if (nMaxDepth < nMinDepth || nMaxDepth < 10) return false;
	nMinDepth = 50000;
	nMaxDepth = nBestHeight;
	int ii = 0;
			for (ii = nMinDepth; ii <= nMaxDepth; ii++)
			{
     			CBlockIndex* pblockindex = FindBlockByHeight(ii);
				if (block.ReadFromDisk(pblockindex))
				{
					int64_t nFees = 0;
					int64_t nValueIn = 0;
					int64_t nValueOut = 0;
					int64_t nStakeReward = 0;
					//unsigned int nSigOps = 0;
					double DPOR_Paid = 0;
					bool bIsDPOR = false;
					std::string MainRecipient = "";
					double max_subsidy = GetMaximumBoincSubsidy(block.nTime)+50; //allow for
					BOOST_FOREACH(CTransaction& tx, block.vtx)
					{

						    MapPrevTx mapInputs;
					        if (tx.IsCoinBase())
									nValueOut += tx.GetValueOut();
							else
							{
									 bool fInvalid;
									 bool TxOK = tx.FetchInputs(txdb, mapQueuedChanges, true, false, mapInputs, fInvalid);
									 if (!TxOK) continue;
									 int64_t nTxValueIn = tx.GetValueIn(mapInputs);
									 int64_t nTxValueOut = tx.GetValueOut();
									 nValueIn += nTxValueIn;
									 nValueOut += nTxValueOut;
									 if (!tx.IsCoinStake())             nFees += nTxValueIn - nTxValueOut;
									 if (tx.IsCoinStake())
				  				 	 {
											nStakeReward = nTxValueOut - nTxValueIn;
											if (tx.vout.size() > 2) bIsDPOR = true;
											//DPOR Verification of each recipient (Recipients start at output position 2 (0=Coinstake flag, 1=coinstake)
											if (tx.vout.size() > 2)
											{
												MainRecipient = PubKeyToAddress(tx.vout[2].scriptPubKey);
											}
											int iStart = 3;
											if (ii > 267500) iStart=2;
											if (bIsDPOR)
											{
													for (unsigned int i = iStart; i < tx.vout.size(); i++)
													{
														std::string Recipient = PubKeyToAddress(tx.vout[i].scriptPubKey);
														double      Amount    = CoinToDouble(tx.vout[i].nValue);
														if (Amount > GetMaximumBoincSubsidy(GetAdjustedTime()))
														{
														}

														if (Amount > max_subsidy)
														{
															printf("Block #%f:%f, Recipient %s, Paid %f\r\n",(double)ii,(double)i,Recipient.c_str(),Amount);
														}
			   	 	  	 							    DPOR_Paid += Amount;

													}

										   }
								     }

								//if (!tx.ConnectInputs(txdb, mapInputs, mapQueuedChanges, posThisTx, pindex, true, false))                return false;
							}

					}

					int64_t TotalMint = nValueOut - nValueIn + nFees;
					double subsidy = CoinToDouble(TotalMint);
					if (subsidy > max_subsidy)
					{
						std::string hb = block.vtx[0].hashBoinc;
						MiningCPID bb = DeserializeBoincBlock(hb);
						if (bb.cpid != "INVESTOR")
						{
								printf("Block #%f:%f, Recipient %s, CPID %s, Paid %f, StakeReward %f \r\n",(double)ii,(double)0,
									bb.GRCAddress.c_str(), bb.cpid.c_str(), subsidy,(double)nStakeReward);
						}
				}

			}
		}


    return true;
}


void TestScan()
{
	BOOST_FOREACH(const PAIRTYPE(uint256, CBlockIndex*)& item, mapBlockIndex)
    {
        CBlockIndex* pindex = item.second;
		if (LessVerbose(1) || pindex->nHeight > nNewIndex2)
		{
			printf("map block index h %f ,  cpid %s   , Mag  %f , RS %f, INT %f \r\n",(double)pindex->nHeight,pindex->sCPID.c_str(), (double)pindex->nMagnitude,
				pindex->nResearchSubsidy,pindex->nInterestSubsidy);
		}
    }
}


void TestScan2()
{
	CBlockIndex* pindex = pindexBest;
    while (pindex->nHeight > 1)
	{
        pindex = pindex->pprev;
		if (LessVerbose(1) || pindex->nHeight > nNewIndex2)
		{
			printf("map block index h %f ,  cpid %s   , Mag  %f , RS %f, INT %f \r\n",(double)pindex->nHeight,pindex->sCPID.c_str(), (double)pindex->nMagnitude,
				pindex->nResearchSubsidy,pindex->nInterestSubsidy);
		}

	}
}

double GRCMagnitudeUnit(int64_t locktime)
{
	//7-12-2015 - Calculate GRCMagnitudeUnit (Amount paid per magnitude per day)
	StructCPID network = GetInitializedStructCPID2("NETWORK",mvNetwork);
	double TotalNetworkMagnitude = network.NetworkMagnitude;
	if (TotalNetworkMagnitude < 1000) TotalNetworkMagnitude=1000;
	double MaximumEmission = BLOCKS_PER_DAY*GetMaximumBoincSubsidy(locktime);
	double Kitty = MaximumEmission - (network.payments/14);
	if (Kitty < 1) Kitty = 1;
	double MagnitudeUnit = 0;
	if (AreBinarySuperblocksEnabled(nBestHeight))
	{
		MagnitudeUnit = (Kitty/TotalNetworkMagnitude)*1.25;
	}
	else
	{
		MagnitudeUnit = Kitty/TotalNetworkMagnitude;
	}
	if (MagnitudeUnit > 5) MagnitudeUnit = 5; //Just in case we lose a superblock or something strange happens.
	MagnitudeUnit = SnapToGrid(MagnitudeUnit); //Snaps the value into .025 increments
	return MagnitudeUnit;
}


int64_t ComputeResearchAccrual(int64_t nTime, std::string cpid, std::string operation, CBlockIndex* pindexLast, bool bVerifyingBlock, int iVerificationPhase, double& dAccrualAge, double& dMagnitudeUnit, double& AvgMagnitude)
{
	double dCurrentMagnitude = CalculatedMagnitude2(cpid, nTime, false);
	CBlockIndex* pHistorical = GetHistoricalMagnitude(cpid);
	if (pHistorical->nHeight <= nNewIndex || pHistorical->nMagnitude==0 || pHistorical->nTime == 0)
	{
		//No prior block exists... Newbies get .01 age to bootstrap the CPID (otherwise they will not have any prior block to refer to, thus cannot get started):
		if (!AreBinarySuperblocksEnabled(pindexLast->nHeight))
		{
				return dCurrentMagnitude > 0 ? ((dCurrentMagnitude/100)*COIN) : 0;
		}
		else
		{
			// New rules - 12-4-2015 - Pay newbie from the moment beacon was sent as long as it is within 6 months old and NN mag > 0 and newbie is in the superblock and their lifetime paid is zero
			// Note: If Magnitude is zero, or researcher is not in superblock, or lifetimepaid > 0, this function returns zero
			int64_t iBeaconTimestamp = BeaconTimeStamp(cpid, true);
			if (IsLockTimeWithinMinutes(iBeaconTimestamp, 60*24*30*6))
			{
				double dNewbieAccrualAge = ((double)nTime - (double)iBeaconTimestamp) / 86400;
				int64_t iAccrual = (int64_t)((dNewbieAccrualAge*dCurrentMagnitude*dMagnitudeUnit*COIN) + (1*COIN));
				if ((dNewbieAccrualAge*dCurrentMagnitude*dMagnitudeUnit) > 500)
				{
					printf("Newbie special stake too high, reward=500GRC");
					return (500*COIN);
				}
				if (fDebug3) printf("\r\n Newbie Special First Stake for CPID %s, Age %f, Accrual %f \r\n",cpid.c_str(),dNewbieAccrualAge,(double)iAccrual);
				return iAccrual;
			}
			else
			{
				return dCurrentMagnitude > 0 ? (((dCurrentMagnitude/100)*COIN) + (1*COIN)): 0;
			}
		}
	}
	// To prevent reorgs and checkblock errors, ensure the research age is > 10 blocks wide:
	int iRABlockSpan = pindexLast->nHeight - pHistorical->nHeight;
	StructCPID stCPID = GetInitializedStructCPID2(cpid,mvResearchAge);
	double dAvgMag = stCPID.ResearchAverageMagnitude;
	// ResearchAge: If the accrual age is > 20 days, add in the midpoint lifetime average magnitude to ensure the overall avg magnitude accurate:
	if (iRABlockSpan > (int)(BLOCKS_PER_DAY*20))
	{
			AvgMagnitude = (pHistorical->nMagnitude + dAvgMag + dCurrentMagnitude) / 3;
	}
	else
	{
			AvgMagnitude = (pHistorical->nMagnitude + dCurrentMagnitude) / 2;
	}
	if (AvgMagnitude > 20000) AvgMagnitude = 20000;

	dAccrualAge = ((double)nTime - (double)pHistorical->nTime) / 86400;
	if (dAccrualAge < 0) dAccrualAge=0;
	dMagnitudeUnit = GRCMagnitudeUnit(nTime);

	int64_t Accrual = (int64_t)(dAccrualAge*AvgMagnitude*dMagnitudeUnit*COIN);
	// Double check researcher lifetime paid
	double days = (((double)nTime) - stCPID.LowLockTime)/86400;
	double PPD = stCPID.ResearchSubsidy/(days+.01);
	double ReferencePPD = dMagnitudeUnit*dAvgMag;
	if ((PPD > ReferencePPD*5))
	{
			printf("Researcher PPD %f > Reference PPD %f for CPID %s with Lifetime Avg Mag of %f, Days %f \r\n",PPD,ReferencePPD,cpid.c_str(),dAvgMag,days);
			Accrual = 0; //Since this condition can occur when a user ramps up computing power, lets return 0 so as to not shortchange the researcher, but instead, owed will continue to accrue and will be paid later when PPD falls below 5
	}
	// Note that if the RA Block Span < 10, we want to return 0 for the Accrual Amount so the CPID can still receive an accurate accrual in the future
	if (iRABlockSpan < 10 && iVerificationPhase != 2) Accrual = 0;

	double verbosity = (operation == "createnewblock" || operation == "createcoinstake") ? 10 : 1000;
	if ((fDebug && LessVerbose(verbosity)) || (fDebug3 && iVerificationPhase==2)) printf(" Operation %s, ComputedAccrual %f, StakeHeight %f, RABlockSpan %f, HistoryHeight%f, AccrualAge %f, AvgMag %f, MagUnit %f, PPD %f, Reference PPD %f  \r\n",
		operation.c_str(),CoinToDouble(Accrual),(double)pindexLast->nHeight,(double)iRABlockSpan,
		(double)pHistorical->nHeight,	dAccrualAge,AvgMagnitude,dMagnitudeUnit, PPD, ReferencePPD);
	return Accrual;
}



CBlockIndex* GetHistoricalMagnitude(std::string cpid)
{
	if (cpid=="INVESTOR") return pindexGenesisBlock;
	// Starting at the block prior to StartHeight, find the last instance of the CPID in the chain:
	// Limit lookback to 6 months
	int nMinIndex = pindexBest->nHeight-(6*30*BLOCKS_PER_DAY);
	if (nMinIndex < 2) nMinIndex=2;
	// Last block Hash paid to researcher
	StructCPID stCPID = GetInitializedStructCPID2(cpid,mvResearchAge);
	if (!stCPID.BlockHash.empty())
	{
		uint256 hash(stCPID.BlockHash);
		if (mapBlockIndex.count(hash) == 0) return pindexGenesisBlock;
		CBlockIndex* pblockindex = mapBlockIndex[hash];
		if ((double)pblockindex->nHeight < nMinIndex)
		{
			// In this case, the last staked block was Found, but it is over 6 months old....
			printf("Last staked block found at height %f, but cannot verify magnitude older than 6 months! \r\n",(double)pblockindex->nHeight);
			return pindexGenesisBlock;
		}

		return pblockindex;
	}
	else
	{
		return pindexGenesisBlock;
	}
}

void ZeroOutResearcherTotals(std::string cpid)
{
	if (!cpid.empty())
	{

				StructCPID stCPID = GetInitializedStructCPID2(cpid,mvResearchAge);
				stCPID.LastBlock = 0;
				stCPID.BlockHash = "";
				stCPID.InterestSubsidy = 0;
				stCPID.ResearchSubsidy = 0;
				stCPID.Accuracy = 0;
				stCPID.LowLockTime = 99999999999;
				stCPID.HighLockTime = 0;
				stCPID.TotalMagnitude = 0;
				stCPID.ResearchAverageMagnitude = 0;

				mvResearchAge[cpid]=stCPID;
	}

}




CBlockIndex* GetHistoricalMagnitude_ScanChain(std::string cpid)
{
	CBlockIndex* pindex = pindexBest;
	if (cpid=="INVESTOR") return pindexGenesisBlock;

	// Starting at the block prior to StartHeight, find the last instance of the CPID in the chain:
	// Limit lookback to 6 months
	int nMinIndex = pindexBest->nHeight-(6*30*BLOCKS_PER_DAY);
	if (nMinIndex < 2) nMinIndex=2;
    while (pindex->nHeight > nNewIndex2 && pindex->nHeight > nMinIndex)
	{
  	    //8-5-2015; R HALFORD; Find the last block the CPID staked with a research subsidy (IE dont count interest blocks)
		if (!pindex || !pindex->pprev) return pindexGenesisBlock;
		if (!pindex->IsInMainChain()) continue;
		if (pindex == pindexGenesisBlock) return pindexGenesisBlock;
		if (pindex->sCPID == cpid && (pindex->nResearchSubsidy > 0)) return pindex;
	    pindex = pindex->pprev;
	}
    return pindexGenesisBlock;
}


bool LoadAdminMessages(bool bFullTableScan, std::string& out_errors)
{
	int nMaxDepth = nBestHeight;
    int nMinDepth = fTestNet ? 1 : 164618;
	nMinDepth = pindexBest->nHeight - (BLOCKS_PER_DAY*30*6);
	if (nMinDepth < 2) nMinDepth=2;
	if (!bFullTableScan) nMinDepth = nMaxDepth-6;
	if (nMaxDepth < nMinDepth) return false;
	CBlockIndex* pindex = pindexBest;
	pindex = FindBlockByHeight(nMinDepth);
	// These are memorized consecutively in order from oldest to newest

    while (pindex->nHeight < nMaxDepth)
	{
		if (!pindex || !pindex->pnext) return false;
	    pindex = pindex->pnext;
		if (pindex==NULL) continue;
		if (!pindex || !pindex->IsInMainChain()) continue;
		if (IsContract(pindex))
		{
			CBlock block;
			if (!block.ReadFromDisk(pindex)) continue;
			int iPos = 0;
			BOOST_FOREACH(const CTransaction &tx, block.vtx)
			{
				  if (iPos > 0)
				  {
					  // Retrieve the Burn Amount for Contracts
					  double dAmount = 0;
					  std::string sRecipient = "";
   					  for (unsigned int i = 1; i < tx.vout.size(); i++)
					  {
							sRecipient = PubKeyToAddress(tx.vout[i].scriptPubKey);
							dAmount += CoinToDouble(tx.vout[i].nValue);
					  }
					  MemorizeMessage(tx.hashBoinc,tx.nTime,dAmount,sRecipient);
				  }
				  iPos++;
			}
		}
	}
	
	return true;
}




MiningCPID GetBoincBlockByIndex(CBlockIndex* pblockindex)
{
	CBlock block;
	MiningCPID bb;
	bb.initialized=false;
	if (!pblockindex || !pblockindex->IsInMainChain()) return bb;
	if (block.ReadFromDisk(pblockindex))
	{
		std::string hashboinc = "";
		if (block.vtx.size() > 0) hashboinc = block.vtx[0].hashBoinc;
		bb = DeserializeBoincBlock(hashboinc);
		bb.initialized=true;
		return bb;
	}
	return bb;
}

std::string CPIDHash(double dMagIn, std::string sCPID)
{
    std::string sMag = RoundToString(dMagIn,0);
	double dMagLength = (double)sMag.length();
	double dExponent = pow(dMagLength,5);
	std::string sMagComponent1 = RoundToString(dMagIn/(dExponent+.01),0);
	std::string sSuffix = RoundToString(dMagLength * dExponent, 0);
    std::string sHash = sCPID + sMagComponent1 + sSuffix;
	//	printf("%s, %s, %f, %f, %s\r\n",sCPID.c_str(), sMagComponent1.c_str(),dMagLength,dExponent,sSuffix.c_str());
    return sHash;
}

std::string GetQuorumHash(std::string data)
{
		//Data includes the Magnitudes, and the Projects:
        std::string sMags = ExtractXML(data,"<MAGNITUDES>","</MAGNITUDES>");
		std::vector<std::string> vMags = split(sMags.c_str(),";");
	    std::string sHashIn = "";
        for (unsigned int x = 0; x < vMags.size(); x++)
		{
			if (vMags[x].length() > 10)
			{
				std::vector<std::string> vRow = split(vMags[x].c_str(),",");
				if (vRow.size() > 0)
				{
                  if (vRow[0].length() > 5)
				  {
						std::string sCPID = vRow[0];
						double dMag = cdbl(vRow[1],0);
                        sHashIn += CPIDHash(dMag, sCPID) + "<COL>";
				   }
				}
			}
		}
		std::string sHash = RetrieveMd5(sHashIn);
		return sHash;
}


std::string getHardwareID()
{
	std::string ele1 = "?";
	#ifdef QT_GUI
	    ele1 = getMacAddress();
	#endif
	ele1 += ":" + getCpuHash();
	ele1 += ":" + getHardDriveSerial();

	std::string hwid = RetrieveMd5(ele1);
	return hwid;
}

static void getCpuid( unsigned int* p, unsigned int ax )
 {
    __asm __volatile
    (   "movl %%ebx, %%esi\n\t"
        "cpuid\n\t"
        "xchgl %%ebx, %%esi"
        : "=a" (p[0]), "=S" (p[1]),
          "=c" (p[2]), "=d" (p[3])
        : "0" (ax)
    );
 }

 std::string getCpuHash()
 {
    std::string n = boost::asio::ip::host_name();
	#ifdef WIN32
		unsigned int cpuinfo[4] = { 0, 0, 0, 0 };
		getCpuid( cpuinfo, 0 );
		unsigned short hash = 0;
		unsigned int* ptr = (&cpuinfo[0]);
		for ( unsigned int i = 0; i < 4; i++ )
			hash += (ptr[i] & 0xFFFF) + ( ptr[i] >> 16 );
		double dHash = (double)hash;
		return n + ";" + RoundToString(dHash,0);
	#else
		return n;
	#endif
 }



std::string SystemCommand(const char* cmd)
{
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe))
	{
    	if(fgets(buffer, 128, pipe) != NULL)
    		result += buffer;
    }
    pclose(pipe);
    return result;
}


std::string getHardDriveSerial()
{
	if (!msHDDSerial.empty()) return msHDDSerial;
	std::string cmd1 = "";
	#ifdef WIN32
		cmd1 = "wmic path win32_physicalmedia get SerialNumber";
	#else
		cmd1 = "ls /dev/disk/by-uuid";
	#endif
	std::string result = SystemCommand(cmd1.c_str());
	//if (fDebug3) printf("result %s",result.c_str());
	msHDDSerial = result;
	return result;
}

std::string GetBlockIndexData(std::string sKey)
{
  	CTxDB txdb;
	std::string sValue = "";
	if (!txdb.ReadGenericData(sKey,sValue)) return "";
	return sValue;
}

bool SetBlockIndexData(std::string sKey, std::string sValue)
{
		CTxDB txdb;
		txdb.TxnBegin();
		if (!txdb.WriteGenericData(sKey,sValue)) return false;
		if (!txdb.TxnCommit()) return false;
		return true;
}

bool IsContract(CBlockIndex* pIndex)
{
	return pIndex->nIsContract==1 ? true : false;
}

bool IsSuperBlock(CBlockIndex* pIndex)
{
	return pIndex->nIsSuperBlock==1 ? true : false;
}

void SetUpExtendedBlockIndexFieldsOnce()
{
	return;

	printf("SETUPExtendedBIfieldsOnce Testnet: %s \r\n",YesNo(fTestNet).c_str());
	if (fTestNet)
	{
		if (pindexBest->nHeight < 20000) return;	}
	else
	{
		if (pindexBest->nHeight < 361873) return;
	}

	std::string sSuperblocks = "";
	std::string sContracts   = "";
    CBlockIndex* pindex = pindexGenesisBlock;
	int iStartHeight = fTestNet ? 20000 : 361873;

	pindex = FindBlockByHeight(iStartHeight);
    if (!pindex) return;

	if (pindex && pindex->pnext)
	{
		while (pindex->nHeight < (nNewIndex2 + 1))
		{
				if (!pindex || !pindex->pnext) break;
				pindex = pindex->pnext;
				if (pindex==NULL || !pindex->IsInMainChain()) continue;
				CBlock block;
				if (!block.ReadFromDisk(pindex)) continue;
	     		MiningCPID bb = DeserializeBoincBlock(block.vtx[0].hashBoinc);
				if (bb.superblock.length() > 20)
				{
						sSuperblocks += pindex->GetBlockHash().GetHex() + ",";
				}

				BOOST_FOREACH(const CTransaction &tx, block.vtx)
				{
						if (tx.hashBoinc.length() > 20)
						{
							bool fMemorized = MemorizeMessage(tx.hashBoinc,tx.nTime,0,"");
							if (fMemorized)
							{
								sContracts += pindex->GetBlockHash().GetHex() + ",";
								break;
							}
						}
				}
		}
	}
}


double SnapToGrid(double d)
{
	double dDither = .04;
	double dOut = cdbl(RoundToString(d*dDither,3),3) / dDither;
	return dOut;
}



bool NeuralNodeParticipates()
{
	//Calculate the nodes GRC_Address_Day
	std::string address_day = DefaultWalletAddress() + "_" + RoundToString(GetDayOfYear(),0);
	std::string address_day_hash = RetrieveMd5(address_day);
	// For now, let's call for a 25% participation rate (approx. 125 nodes):
	// When RA is enabled, 25% of the neural network nodes will work on a quorum at any given time to alleviate stress on the project sites:
	uint256 uRef;
	if (IsResearchAgeEnabled(pindexBest->nHeight))
	{
		uRef = fTestNet ? uint256("0x00000000000000000000000000000000ed182f81388f317df738fd9994e7020b") : uint256("0x000000000000000000000000000000004d182f81388f317df738fd9994e7020b"); //This hash is approx 25% of the md5 range (90% for testnet)
	}
	else
	{
		uRef = fTestNet ? uint256("0x00000000000000000000000000000000ed182f81388f317df738fd9994e7020b") : uint256("0x00000000000000000000000000000000fd182f81388f317df738fd9994e7020b"); //This hash is approx 25% of the md5 range (90% for testnet)
	}
	uint256 uADH = uint256("0x" + address_day_hash);
	//printf("%s < %s : %s",uADH.GetHex().c_str() ,uRef.GetHex().c_str(), YesNo(uADH  < uRef).c_str());
	//printf("%s < %s : %s",uTest.GetHex().c_str(),uRef.GetHex().c_str(), YesNo(uTest < uRef).c_str());
	return (uADH < uRef);
}


bool StrLessThanReferenceHash(std::string rh)
{
	std::string address_day = rh + "_" + RoundToString(GetDayOfYear(),0);
	std::string address_day_hash = RetrieveMd5(address_day);
	uint256 uRef = fTestNet ? uint256("0x000000000000000000000000000000004d182f81388f317df738fd9994e7020b") : uint256("0x000000000000000000000000000000004d182f81388f317df738fd9994e7020b"); //This hash is approx 25% of the md5 range (90% for testnet)
	uint256 uADH = uint256("0x" + address_day_hash);
	return (uADH < uRef);
}


int64_t CoinFromValue(double dAmount)
{
    if (dAmount <= 0.0 || dAmount > MAX_MONEY)        throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount");
	int64_t nAmount = roundint64(dAmount * COIN);
    if (!MoneyRange(nAmount))                         throw JSONRPCError(RPC_TYPE_ERROR, "Invalid amount");
    return nAmount;
}




