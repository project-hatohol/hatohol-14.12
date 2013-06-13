#include <cppcutter.h>
#include <cutter.h>

#include "Asura.h"
#include "DBClientAsura.h"
#include "ConfigManager.h"
#include "Helpers.h"
#include "DBClientTest.h"

namespace testDBClientAsura {

static void addTriggerInfo(TriggerInfo *triggerInfo)
{
	DBClientAsura dbAsura;
	dbAsura.addTriggerInfo(triggerInfo);
}
#define assertAddTriggerToDB(X) \
cut_trace(_assertAddToDB<TriggerInfo>(X, addTriggerInfo))

static string makeExpectedOutput(TriggerInfo *triggerInfo)
{
	string expectedOut = StringUtils::sprintf
	                       ("%u|%llu|%d|%d|%d|%lu|%llu|%s|%s\n",
	                        triggerInfo->serverId,
	                        triggerInfo->id,
	                        triggerInfo->status, triggerInfo->severity,
	                        triggerInfo->lastChangeTime.tv_sec,
	                        triggerInfo->lastChangeTime.tv_nsec,
	                        triggerInfo->hostId,
	                        triggerInfo->hostName.c_str(),
	                        triggerInfo->brief.c_str());
	return expectedOut;
}

static void _assertGetTriggers(void)
{
	TriggerInfoList triggerInfoList;
	DBClientAsura dbAsura;
	dbAsura.getTriggerInfoList(triggerInfoList);
	cppcut_assert_equal(NumTestTriggerInfo, triggerInfoList.size());

	string expectedText;
	string actualText;
	TriggerInfoListIterator it = triggerInfoList.begin();
	for (size_t i = 0; i < NumTestTriggerInfo; i++, ++it) {
		expectedText += makeExpectedOutput(&testTriggerInfo[i]);
		actualText += makeExpectedOutput(&(*it));
	}
	cppcut_assert_equal(expectedText, actualText);
}
#define assertGetTriggers() cut_trace(_assertGetTriggers())

// TODO: The names of makeExpectedOutput() and makeExpectedItemOutput()
//       will be changed to be the similar of this function.
static string makeEventOutput(EventInfo &eventInfo)
{
	string output = StringUtils::sprintf
	                  ("%u|%llu|%ld|%ld|%d|%u|%llu|%s|%s\n",
	                   eventInfo.serverId, eventInfo.id,
	                   eventInfo.time.tv_sec, eventInfo.time.tv_nsec,
	                   eventInfo.status, eventInfo.severity,
	                   eventInfo.hostId,
	                   eventInfo.hostName.c_str(),
	                   eventInfo.brief.c_str());
	return output;
}

static void _assertGetEvents(void)
{
	EventInfoList eventInfoList;
	DBClientAsura dbAsura;
	dbAsura.getEventInfoList(eventInfoList);
	cppcut_assert_equal(NumTestEventInfo, eventInfoList.size());

	string expectedText;
	string actualText;
	EventInfoListIterator it = eventInfoList.begin();
	for (size_t i = 0; i < NumTestEventInfo; i++, ++it) {
		expectedText += makeEventOutput(testEventInfo[i]);
		actualText += makeEventOutput(*it);
	}
	cppcut_assert_equal(expectedText, actualText);
}
#define assertGetEvents() cut_trace(_assertGetEvents())

static string makeExpectedItemOutput(ItemInfo *itemInfo)
{
	string expectedOut = StringUtils::sprintf
	                       ("%u|%llu|%d|%d|%d|%lu|%llu|%s|%s\n",
	                        itemInfo->serverId,
	                        itemInfo->id,
	                        itemInfo->hostId,
	                        itemInfo->brief.c_str(),
	                        itemInfo->lastValueTime.tv_sec,
	                        itemInfo->lastValueTime.tv_nsec,
	                        itemInfo->lastValue.c_str(),
	                        itemInfo->prevValue.c_str(),
	                        itemInfo->itemGroupName.c_str());
	return expectedOut;
}

static void _assertGetItems(void)
{
	ItemInfoList itemInfoList;
	DBClientAsura dbAsura;
	dbAsura.getItemInfoList(itemInfoList);
	cppcut_assert_equal(NumTestItemInfo, itemInfoList.size());

	string expectedText;
	string actualText;
	ItemInfoListIterator it = itemInfoList.begin();
	for (size_t i = 0; i < NumTestItemInfo; i++, ++it) {
		expectedText += makeExpectedItemOutput(&testItemInfo[i]);
		actualText += makeExpectedItemOutput(&(*it));
	}
	cppcut_assert_equal(expectedText, actualText);
}
#define assertGetItems() cut_trace(_assertGetItems())

void setup(void)
{
	asuraInit();
}

// ---------------------------------------------------------------------------
// Test cases
// ---------------------------------------------------------------------------
void test_createDB(void)
{
	// remove the DB that already exists
	string dbPath = deleteDBClientDB(DB_DOMAIN_ID_ASURA);

	// create an instance (the database will be automatically created)
	DBClientAsura dbAsura;
	cut_assert_exist_path(dbPath.c_str());

	// check the version
	string statement = "select * from _dbclient";
	string output = execSqlite3ForDBClient(DB_DOMAIN_ID_ASURA, statement);
	string expectedOut = StringUtils::sprintf
	                       ("%d|%d\n", DBClient::DBCLIENT_DB_VERSION,
	                                   DBClientAsura::ASURA_DB_VERSION);
	cppcut_assert_equal(expectedOut, output);
}

void test_createTableTrigger(void)
{
	const string tableName = "triggers";
	string dbPath = deleteDBClientDB(DB_DOMAIN_ID_ASURA);
	DBClientAsura dbAsura;
	string command = "sqlite3 " + dbPath + " \".table\"";
	assertExist(tableName, executeCommand(command));

	// check content
	string statement = "select * from " + tableName;
	string output = execSqlite3ForDBClient(DB_DOMAIN_ID_ASURA, statement);
	string expectedOut = StringUtils::sprintf(""); // currently no data
	cppcut_assert_equal(expectedOut, output);
}

void test_testAddTriggerInfo(void)
{
	string dbPath = deleteDBClientDB(DB_DOMAIN_ID_ASURA);

	// added a record
	TriggerInfo *testInfo = testTriggerInfo;
	assertAddTriggerToDB(testInfo);

	// confirm with the command line tool
	string cmd = StringUtils::sprintf(
	               "sqlite3 %s \"select * from triggers\"", dbPath.c_str());
	string result = executeCommand(cmd);
	string expectedOut = makeExpectedOutput(testInfo);
	cppcut_assert_equal(expectedOut, result);
}

void test_testGetTriggerInfoList(void)
{
	for (size_t i = 0; i < NumTestTriggerInfo; i++)
		assertAddTriggerToDB(&testTriggerInfo[i]);
	assertGetTriggers();
}

void test_setTriggerInfoList(void)
{
	deleteDBClientDB(DB_DOMAIN_ID_ASURA);

	DBClientAsura dbAsura;
	TriggerInfoList triggerInfoList;
	for (size_t i = 0; i < NumTestTriggerInfo; i++)
		triggerInfoList.push_back(testTriggerInfo[i]);
	uint32_t serverId = testTriggerInfo[0].serverId;
	dbAsura.setTriggerInfoList(triggerInfoList, serverId);

	assertGetTriggers();
}

void test_addTriggerInfoList(void)
{
	deleteDBClientDB(DB_DOMAIN_ID_ASURA);

	size_t i;
	DBClientAsura dbAsura;

	// First call
	size_t numFirstAdd = NumTestTriggerInfo / 2;
	TriggerInfoList triggerInfoList0;
	for (i = 0; i < numFirstAdd; i++)
		triggerInfoList0.push_back(testTriggerInfo[i]);
	dbAsura.addTriggerInfoList(triggerInfoList0);

	// Second call
	TriggerInfoList triggerInfoList1;
	for (; i < NumTestTriggerInfo; i++)
		triggerInfoList1.push_back(testTriggerInfo[i]);
	dbAsura.addTriggerInfoList(triggerInfoList1);

	// Check
	assertGetTriggers();
}

void test_addItemInfoList(void)
{
	deleteDBClientDB(DB_DOMAIN_ID_ASURA);

	DBClientAsura dbAsura;
	ItemInfoList itemInfoList;
	for (size_t i = 0; i < NumTestItemInfo; i++)
		itemInfoList.push_back(testItemInfo[i]);
	dbAsura.addItemInfoList(itemInfoList);

	assertGetItems();
}

void test_addEventInfoList(void)
{
	// DBClientAsura internally joins the trigger table and the event table.
	// So we also have to add trigger data.
	// When the internal join is removed, the following line will not be
	// needed.
	test_setTriggerInfoList();

	DBClientAsura dbAsura;
	EventInfoList eventInfoList;
	for (size_t i = 0; i < NumTestEventInfo; i++)
		eventInfoList.push_back(testEventInfo[i]);
	dbAsura.addEventInfoList(eventInfoList);

	assertGetEvents();
}

void test_getLastEventId(void)
{
	test_addEventInfoList();
	DBClientAsura dbAsura;
	const int serverid = 3;
	cppcut_assert_equal(findLastEventId(serverid),
	                    dbAsura.getLastEventId(serverid));
}

} // namespace testDBClientAsura
