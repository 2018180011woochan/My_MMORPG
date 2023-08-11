#include "pch.h"
#include "SectorManager.h"
#include "SessionManager.h"

SectorManager GSectorManager;

void SectorManager::Init()
{
	for (int i = 0; i < SectorAllCount; ++i) 
		_Sector[i] = new Sector(i);
}

void SectorManager::PushSector(int _id)
{
	int preSecID = GSessionManager.clients[_id]._ObjStat.SectorID;

	int tx = GSessionManager.clients[_id]._ObjStat.x / 250;		
	int ty = GSessionManager.clients[_id]._ObjStat.y / 250;		

	int SecID = (ty * SectorCount) + tx;

	if (preSecID != SecID) {
		_Sector[preSecID]->SectorSubObject(_id);
	}

	_Sector[SecID]->SectorAddObject(_id);
	GSessionManager.clients[_id]._ObjStat.SectorID = SecID;
	if (GSessionManager.clients[_id]._ObjStat.Race == RACE::RACE_PLAYER)
		PushPlayerViewList(_id);
}

void SectorManager::PopSector(int _id)
{
	int tx = GSessionManager.clients[_id]._ObjStat.x / 250;		
	int ty = GSessionManager.clients[_id]._ObjStat.y / 250;		

	int SecID = (ty * SectorCount) + tx;
	_Sector[SecID]->SectorSubObject(_id);
	PopPlayerViewList(_id);
}

void SectorManager::PushPlayerViewList(int _id)
{
	unordered_set<int> NewViewList;
	// 해당 섹터와 상하좌우 8구역 
	SetViewListMove(_id, GSessionManager.clients[_id]._ObjStat.SectorID, NewViewList);	// 해당 섹터 뷰리스트 추가

	// 좌상
	int secID = GSessionManager.clients[_id]._ObjStat.SectorID - SectorCount - 1;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 상
	secID = GSessionManager.clients[_id]._ObjStat.SectorID - SectorCount;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 우상
	secID = GSessionManager.clients[_id]._ObjStat.SectorID - SectorCount + 1;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 좌
	secID = GSessionManager.clients[_id]._ObjStat.SectorID - 1;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 우
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + 1;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 좌하
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + SectorCount - 1;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 하
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + SectorCount;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);
	// 우하
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + SectorCount + 1;
	if (isValidSector(secID))
		SetViewListMove(_id, secID, NewViewList);

}

void SectorManager::PopPlayerViewList(int _id)
{
	
}

void SectorManager::SetViewListMove(int _playerID, int _sectorID, unordered_set<int>& _new_vl)
{
	unordered_set<int> sectorViewList = _Sector[_sectorID]->GetSectorPlayers();

	GSessionManager.clients[_playerID]._ViewListLock.lock();
	unordered_set<int> old_vl = GSessionManager.clients[_playerID]._ViewList;
	GSessionManager.clients[_playerID]._ViewListLock.unlock();

	//unordered_set<int> new_vl;
	for (auto& id : sectorViewList) {
		//if (ST_INGAME != GSessionManager.clients[id]._SessionState) continue;
		if (_playerID == id) continue;

		if (RANGE > GSessionManager.Distance(_playerID, id))
			_new_vl.insert(id);
	}
	GSessionManager.clients[_playerID].SendMovePacket(_playerID, 0);

	for (auto pl : _new_vl) {
		// old_vl에 없는데 new_vl에 있으면 add패킷 보내기
		if (0 == old_vl.count(pl)) {
			GSessionManager.clients[_playerID].SendAddObjectPacket(pl);
			GSessionManager.clients[_playerID]._ViewListLock.lock();
			GSessionManager.clients[_playerID]._ViewList.insert(pl);
			GSessionManager.clients[_playerID]._ViewListLock.unlock();
			GSessionManager.clients[pl]._SessionState = ST_INGAME;

			if (GSessionManager.clients[pl]._ObjStat.Race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
				continue;
			GSessionManager.clients[pl]._ViewListLock.lock();
			if (0 == GSessionManager.clients[pl]._ViewList.count(_playerID)) {
				GSessionManager.clients[pl].SendAddObjectPacket(_playerID);
				GSessionManager.clients[pl]._ViewList.insert(_playerID);
				GSessionManager.clients[pl]._ViewListLock.unlock();
				GSessionManager.clients[pl]._SessionState = ST_INGAME;
			}
			else {
				GSessionManager.clients[pl]._ViewListLock.unlock();
				GSessionManager.clients[pl].SendMovePacket(_playerID, 0);
			}
		}
		// old_vl에도 있고 new_vl에도 있으면 move패킷 보내기
		else {
			if (GSessionManager.clients[pl]._ObjStat.Race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
				continue;

			GSessionManager.clients[pl]._ViewListLock.lock();
			if (0 == GSessionManager.clients[pl]._ViewList.count(_playerID)) {
				GSessionManager.clients[pl].SendAddObjectPacket(_playerID);
				GSessionManager.clients[pl]._ViewList.insert(_playerID);
				GSessionManager.clients[pl]._ViewListLock.unlock();
				GSessionManager.clients[pl]._SessionState = ST_INGAME;
			}
			else {
				GSessionManager.clients[pl]._ViewListLock.unlock();
				GSessionManager.clients[pl].SendMovePacket(_playerID, 0);
			}
		}
	}

	for (auto pl : old_vl) {
		// old에는 있는데 new에는 없으면 삭제
		if (0 == _new_vl.count(pl)) {
			GSessionManager.clients[_playerID].SendRemovePacket(pl);
			GSessionManager.clients[_playerID]._ViewListLock.lock();
			GSessionManager.clients[_playerID]._ViewList.erase(pl);
			GSessionManager.clients[_playerID]._ViewListLock.unlock();
			GSessionManager.clients[pl]._SessionState = ST_SLEEP;

			if (GSessionManager.clients[pl]._ObjStat.Race != RACE::RACE_PLAYER)	// player가 아니라면 패킷을 보낼 필요가 없다
				continue;
			GSessionManager.clients[pl]._ViewListLock.lock();
			if (0 == GSessionManager.clients[pl]._ViewList.count(_playerID)) {
				GSessionManager.clients[pl]._ViewListLock.unlock();
			}
			else {
				GSessionManager.clients[pl].SendRemovePacket(_playerID);
				GSessionManager.clients[pl]._ViewList.erase(_playerID);
				GSessionManager.clients[pl]._ViewListLock.unlock();
				GSessionManager.clients[pl]._SessionState = ST_SLEEP;
			}
		}
	}
}

bool SectorManager::isInSector(int _cid, int _nid)
{
	int nid = GSessionManager.clients[_nid]._ObjStat.SectorID;
	if (nid == (GSessionManager.clients[_cid]._ObjStat.SectorID || GSessionManager.clients[_cid]._ObjStat.SectorID - SectorCount - 1 ||
		GSessionManager.clients[_cid]._ObjStat.SectorID - SectorCount || GSessionManager.clients[_cid]._ObjStat.SectorID - SectorCount + 1 ||
		GSessionManager.clients[_cid]._ObjStat.SectorID - 1 || GSessionManager.clients[_cid]._ObjStat.SectorID + 1 ||
		GSessionManager.clients[_cid]._ObjStat.SectorID + SectorCount - 1 || GSessionManager.clients[_cid]._ObjStat.SectorID + SectorCount ||
		GSessionManager.clients[_cid]._ObjStat.SectorID + SectorCount + 1))
		return true;
	return false;
}

bool SectorManager::isValidSector(int _SecID)
{
	if (_SecID < 0 || _SecID >= SectorAllCount)
		return false;
	return true;
}
