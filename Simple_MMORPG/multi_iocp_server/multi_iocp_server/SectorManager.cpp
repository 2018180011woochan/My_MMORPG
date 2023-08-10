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
	int tx = GSessionManager.clients[_id]._ObjStat.x / 250;		
	int ty = GSessionManager.clients[_id]._ObjStat.y / 250;		

	int SecID = (ty * SectorCount) + tx;

	_Sector[SecID]->SectorAddPlayer(_id);
	GSessionManager.clients[_id]._ObjStat.SectorID = SecID;
	PushPlayerViewList(_id);
}

void SectorManager::PopSector(int _id)
{
	int tx = GSessionManager.clients[_id]._ObjStat.x / 250;		
	int ty = GSessionManager.clients[_id]._ObjStat.y / 250;		

	int SecID = (ty * SectorCount) + tx;
	_Sector[SecID]->SectorSubPlayer(_id);
	PopPlayerViewList(_id);
}

void SectorManager::PushPlayerViewList(int _id)
{
	// �ش� ���Ϳ� �����¿� 8���� 
	SetViewList(_id, GSessionManager.clients[_id]._ObjStat.SectorID);	// �ش� ���� �丮��Ʈ �߰�

	// �»�
	int secID = GSessionManager.clients[_id]._ObjStat.SectorID - SectorCount - 1;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ��
	secID = GSessionManager.clients[_id]._ObjStat.SectorID - SectorCount;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ���
	secID = GSessionManager.clients[_id]._ObjStat.SectorID - SectorCount + 1;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ��
	secID = GSessionManager.clients[_id]._ObjStat.SectorID - 1;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ��
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + 1;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ����
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + SectorCount - 1;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ��
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + SectorCount;
	if (isValidSector(secID))
		SetViewList(_id, secID);
	// ����
	secID = GSessionManager.clients[_id]._ObjStat.SectorID + SectorCount + 1;
	if (isValidSector(secID))
		SetViewList(_id, secID);

}

void SectorManager::PopPlayerViewList(int _id)
{
}

void SectorManager::SetViewList(int _playerID, int _sectorID)
{
	unordered_set<int> sectorViewList = _Sector[_sectorID]->GetSectorPlayers();

	GSessionManager.clients[_playerID]._ViewListLock.lock();
	unordered_set<int> old_vl = GSessionManager.clients[_playerID]._ViewList;
	GSessionManager.clients[_playerID]._ViewListLock.unlock();

	unordered_set<int> new_vl;
	/*for (int i = 0; i < MAX_USER + NUM_NPC; ++i) {
		if (GSessionManager.clients[i]._ObjStat.Sector !=
			GSessionManager.clients[c_id]._ObjStat.Sector) continue;

		if (ST_INGAME != GSessionManager.clients[i]._SessionState) continue;
		if (c_id == GSessionManager.clients[i]._ObjStat.ID) continue;

		if (RANGE > GSessionManager.Distance(c_id, i))
			new_vl.insert(i);
	}*/
}

bool SectorManager::isValidSector(int _SecID)
{
	if (_SecID < 0 || _SecID >= SectorAllCount)
		return false;
	return true;
}
