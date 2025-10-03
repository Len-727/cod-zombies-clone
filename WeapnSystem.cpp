#include "WeaponSystem.h"

WeaponSystem::WeaponSystem() :
	m_currentWeapon(WeaponType::PISTOL),	//	現在装備中の武器(最初はピストル)
	m_primaryWeapon(WeaponType::PISTOL),	//	プライマリスロット(最初はピストル)
	m_secondaryWeapon(WeaponType::PISTOL),	//	セカンダリスロット(未使用だが、ピストルで初期化)
	m_currentWeaponSlot(0),					//	現在選択中のスロット(0　= プライマリ)
	m_hasSecondaryWeapon(false),			//	セカンダリ武器を持っているか
	m_currentAmmo(8),						//	現在の装填弾数
	m_maxAmmo(8),							//	最大装填可能数
	m_reserveAmmo(80)						//	予備団択
{
	InitializeWeapons();	//	武器データベースを構築
}

void WeaponSystem::InitializeWeapons()
{
	//	武器データ設定
	m_weaponStats[WeaponType::PISTOL] = {
		WeaponType::PISTOL, 30, 8, 80, 0.2f, 50.0f, 1, 1.5f, 0
	};
	m_weaponStats[WeaponType::SHOTGUN] = {
		WeaponType::SHOTGUN, 200, 2, 60, 0.8f, 10.0f, 1, 2.5f, 500
	};
	m_weaponStats[WeaponType::RIFLE] = {
		WeaponType::RIFLE, 50, 20, 180, 0.1f, 100.0f, 2, 2.0f, 1000
	};
	m_weaponStats[WeaponType::SNIPER] = {
		WeaponType::SNIPER, 500, 5, 50, 1.2f, 200.0f, 5, 3.0f, 1500
	};

	//	各武器の初期弾薬状態を設定
	m_weaponAmmoStatus[WeaponType::PISTOL] = { 8, 80 };
	m_weaponAmmoStatus[WeaponType::SHOTGUN] = { 2, 60 };
	m_weaponAmmoStatus[WeaponType::RIFLE] = { 20, 180 };
	m_weaponAmmoStatus[WeaponType::SNIPER] = { 5, 50 };
}

void WeaponSystem::SwitchWeapon(WeaponType newWeapon)
{
	//	現在の武器の弾薬を保存
	m_weaponAmmoStatus[m_currentWeapon] = { m_currentAmmo, m_reserveAmmo };

	//	新しい武器に切り替え
	m_currentWeapon = newWeapon;
	auto& weapon = m_weaponStats[newWeapon];

	//	保存されている弾薬を復元
	m_currentAmmo = m_weaponAmmoStatus[newWeapon].currentAmmo;
	m_reserveAmmo = m_weaponAmmoStatus[newWeapon].reserveAmmo;
	m_maxAmmo = weapon.maxAmmo;
}

bool WeaponSystem::BuyWeapon(WeaponType weaponType, int& playerPoints)
{
	//	今持っている武器と同じなら、弾薬を満タンにする
	if (m_currentWeapon == weaponType)
	{
		int cost = m_weaponStats[weaponType].cost / 2;
		if (playerPoints >= cost)
		{
			playerPoints -= cost;
			m_reserveAmmo = m_weaponStats[weaponType].reserveAmmo;
			m_weaponAmmoStatus[m_currentWeapon].reserveAmmo = m_reserveAmmo;
			return true;
		}
		return false;
	}

	//	すでに所持している武器なら切り替え
	if (m_primaryWeapon == weaponType ||
		(m_hasSecondaryWeapon && m_secondaryWeapon == weaponType))
	{
		SwitchWeapon(weaponType);
		return true;
	}

	//	新規購入
	int cost = m_weaponStats[weaponType].cost;
	if (playerPoints < cost)
		return false;

	playerPoints -= cost;

	if (m_currentWeaponSlot == 0)
		m_primaryWeapon = weaponType;
	else
		m_secondaryWeapon = weaponType;

	if (!m_hasSecondaryWeapon)
	{
		m_secondaryWeapon = WeaponType::PISTOL;
		m_hasSecondaryWeapon = true;
	}

	SwitchWeapon(weaponType);
	return true;
}

void WeaponSystem::UpdateAmmo(int current, int reserve)
{
	m_currentAmmo = current;
	m_reserveAmmo = reserve;
	m_weaponAmmoStatus[m_currentWeapon] = { current, reserve };
}