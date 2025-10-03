#include "WeaponSystem.h"

WeaponSystem::WeaponSystem() :
	m_currentWeapon(WeaponType::PISTOL),	//	���ݑ������̕���(�ŏ��̓s�X�g��)
	m_primaryWeapon(WeaponType::PISTOL),	//	�v���C�}���X���b�g(�ŏ��̓s�X�g��)
	m_secondaryWeapon(WeaponType::PISTOL),	//	�Z�J���_���X���b�g(���g�p�����A�s�X�g���ŏ�����)
	m_currentWeaponSlot(0),					//	���ݑI�𒆂̃X���b�g(0�@= �v���C�}��)
	m_hasSecondaryWeapon(false),			//	�Z�J���_������������Ă��邩
	m_currentAmmo(8),						//	���݂̑��U�e��
	m_maxAmmo(8),							//	�ő呕�U�\��
	m_reserveAmmo(80)						//	�\���c��
{
	InitializeWeapons();	//	����f�[�^�x�[�X���\�z
}

void WeaponSystem::InitializeWeapons()
{
	//	����f�[�^�ݒ�
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

	//	�e����̏����e���Ԃ�ݒ�
	m_weaponAmmoStatus[WeaponType::PISTOL] = { 8, 80 };
	m_weaponAmmoStatus[WeaponType::SHOTGUN] = { 2, 60 };
	m_weaponAmmoStatus[WeaponType::RIFLE] = { 20, 180 };
	m_weaponAmmoStatus[WeaponType::SNIPER] = { 5, 50 };
}

void WeaponSystem::SwitchWeapon(WeaponType newWeapon)
{
	//	���݂̕���̒e���ۑ�
	m_weaponAmmoStatus[m_currentWeapon] = { m_currentAmmo, m_reserveAmmo };

	//	�V��������ɐ؂�ւ�
	m_currentWeapon = newWeapon;
	auto& weapon = m_weaponStats[newWeapon];

	//	�ۑ�����Ă���e��𕜌�
	m_currentAmmo = m_weaponAmmoStatus[newWeapon].currentAmmo;
	m_reserveAmmo = m_weaponAmmoStatus[newWeapon].reserveAmmo;
	m_maxAmmo = weapon.maxAmmo;
}

bool WeaponSystem::BuyWeapon(WeaponType weaponType, int& playerPoints)
{
	//	�������Ă��镐��Ɠ����Ȃ�A�e��𖞃^���ɂ���
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

	//	���łɏ������Ă��镐��Ȃ�؂�ւ�
	if (m_primaryWeapon == weaponType ||
		(m_hasSecondaryWeapon && m_secondaryWeapon == weaponType))
	{
		SwitchWeapon(weaponType);
		return true;
	}

	//	�V�K�w��
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