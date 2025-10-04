#pragma once
#include <DirectXMath.h>

//	�Q�[���̏��
enum class GameState {
	TITLE,
	PLAYING,
	GAMEOVER
};

//	����̎��
enum class WeaponType {
	PISTOL,
	SHOTGUN,
	RIFLE,
	SNIPER
};

//	����f�[�^
struct WeaponData {
	WeaponType type;
	int damage;
	int maxAmmo;
	int reserveAmmo;
	float fireRate;
	float range;
	int penetration;
	float reloadTime;
	int cost;
};

//	����̒e����
struct WeaponAmmo {
	int currentAmmo;
	int reserveAmmo;
};

//	�G
struct Enemy {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT4 color;
	bool isAlive;
	float moveTimer;
	float nextDirectionChange;
	int health;
	int maxHealth;
};

//	�p�[�e�B�N��
struct Particle {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT4 color;
	float lifetime;
	float maxLifetime;
};