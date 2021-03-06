/* --- INCLUDES ------------------------------------------------------------- */
#include "..\Utils\Control.h"


/* --- DEFINES -------------------------------------------------------------- */
#define CONTROL_ALLNODES_KEYWORD           _T("")

#ifdef CONTROL_OUTFILE_HEADER
#undef CONTROL_OUTFILE_HEADER
#define CONTROL_OUTFILE_HEADER      {_T("name:ID"),_T("objectClass:LABEL"),_T("empty:IGNORE")}
#endif


/* --- TYPES -------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static void CallbackBuildDnCache(
	_In_ CSV_HANDLE hOutfile,
	_In_ CSV_HANDLE hDenyOutfile,
	_In_ LPTSTR *tokens
) {
	BOOL bResult = FALSE;
	CACHE_OBJECT_BY_DN cacheEntry = { 0 };
	PCACHE_OBJECT_BY_DN inserted = NULL;
	BOOL newElement = FALSE;

	UNREFERENCED_PARAMETER(hDenyOutfile);

	if (STR_EMPTY(tokens[LdpListDn]) || STR_EMPTY(tokens[LdpListObjectClass]))
		return;

	cacheEntry.dn = _tcsdup(tokens[LdpListDn]);
	if (!cacheEntry.dn)
		FATAL(_T("Could not dup dn <%s>"), tokens[LdpListDn]);

	cacheEntry.objectClass = _tcsdup(tokens[LdpListObjectClass]);
	if (!cacheEntry.objectClass)
		FATAL(_T("Could not dup objectClass <%s>"), tokens[LdpListObjectClass]);

	CacheEntryInsert(
		ppCache,
		(PVOID)&cacheEntry,
		sizeof(CACHE_OBJECT_BY_DN),
		&inserted,
		&newElement
	);
	if (!inserted) {
		LOG(Err, _T("cannot insert new object-by-dn cache entry <%s>"), tokens[LdpListDn]);
	}
	else if (!newElement) {
		LOG(Dbg, _T("object-by-dn cache entry is not new <%s>"), tokens[LdpListDn]);
		free(cacheEntry.dn);
		free(cacheEntry.objectClass);
	}
	else {
		bResult = ControlWriteOutline(hOutfile, tokens[LdpListDn], tokens[LdpListObjectClass], CONTROL_ALLNODES_KEYWORD);
		if (!bResult)
			LOG(Err, _T("Cannot write outline for <%s>"), tokens[LdpListDn]);
	}
}

static void CallbackMakeAllNodes(
	_In_ CSV_HANDLE hOutfile,
	_In_ CSV_HANDLE hDenyOutfile,
	_In_ LPTSTR *tokens
) {
	BOOL bResult = FALSE;
	CACHE_OBJECT_BY_DN cacheEntry = { 0 };
	PCACHE_OBJECT_BY_DN inserted = NULL;
	BOOL newElement = FALSE;
	DWORD i = 0;

	UNREFERENCED_PARAMETER(hDenyOutfile);

	if (STR_EMPTY(tokens[RelDnMaster]) || STR_EMPTY(tokens[RelDnSlave]))
		return;

	for (i = 0; i < 2; i++) {
		cacheEntry.dn = _tcsdup(tokens[i]);
		if (!cacheEntry.dn)
			FATAL(_T("Could not dup dn <%s>"), tokens[i]);
		cacheEntry.objectClass = _tcsdup(_T("foreignobject;unknown"));
		if (!cacheEntry.objectClass)
			FATAL(_T("Could not dup objectclass for dn <%s>"), tokens[i]);

		CacheEntryInsert(
			ppCache,
			(PVOID)&cacheEntry,
			sizeof(CACHE_OBJECT_BY_DN),
			&inserted,
			&newElement
		);
		if (!inserted) {
			LOG(Err, _T("cannot insert new object-by-dn cache entry <%s>"), tokens[i]);
		}
		else if (!newElement) {
			LOG(Dbg, _T("object-by-dn cache entry is not new <%s>"), tokens[i]);
			free(cacheEntry.dn);
			free(cacheEntry.objectClass);
		}
		else {
			LOG(Dbg, _T("successfully inserted new object-by-dn entry for <%d>, writing to file"), tokens[i]);
			bResult = ControlWriteOutline(hOutfile, cacheEntry.dn, cacheEntry.objectClass, CONTROL_ALLNODES_KEYWORD);
		}
	}
}


/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
int _tmain(
	_In_ int argc,
	_In_ TCHAR * argv[]
) {
	PTCHAR outfileHeader[OUTFILE_TOKEN_COUNT] = CONTROL_OUTFILE_HEADER;
	PTCHAR ptName = _T("DNCACHE");

	bCacheBuilt = FALSE;
	CacheCreate(
		&ppCache,
		ptName,
		pfnCompareDn
	);
	bCacheBuilt = FALSE;
	ControlMainForeachCsvResult(argc, argv, outfileHeader, CallbackBuildDnCache, GenericUsage);
	bCacheBuilt = TRUE;
	ControlMainForeachCsvResult(argc, argv, outfileHeader, CallbackMakeAllNodes, GenericUsage);

	return EXIT_SUCCESS;
}
