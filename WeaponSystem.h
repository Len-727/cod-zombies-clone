#pragma once
#include "Entities.h"
#include <map>

class WeaponSystem {
public:
	WeaponSystem();

	void InitializeWeapons();
	void SwitchWeapon(WeaponType newWeapon);
	bool BuyWeapon(WeaponType weaponType, int& playerPoints);
	void UpdateAmmo(int current, int reserve);


	//	ÉQÉbÉ^Å[
	WeaponType GetCurrentWeapon() const { return m_currentWeapon; }
	int GetCurrentAmmo() const { return m_currentAmmo; }
	int GetReserveAmmo() const { return m_reserveAmmo; }
	int GetMaxAmmo() const { return m_maxAmmo; }
	const WeaponData& GetWeaponData(WeaponType type) { return m_weaponStats[type]; }

private:
	std::map<WeaponType, WeaponData> m_weaponStats;
	std::map<WeaponType, WeaponAmmo> m_weaponAmmoStatus;

	WeaponType m_currentWeapon;
	WeaponType m_primaryWeapon;
	WeaponType m_secondaryWeapon;
	int m_currentWeaponSlot;
	bool m_hasSecondaryWeapon;

	int m_currentAmmo;
	int m_maxAmmo;
	int m_reserveAmmo;
};