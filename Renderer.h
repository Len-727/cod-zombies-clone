#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include<wrl/client.h>
#include <memory>
#include <vector>
#include "Entities.h"

namespace DirectX {
	class CommonStates;
	class BasicEffect;
	class SpriteBatch;
	class GeometricPrimitive;
	template<typename T> class PrimitiveBatch;
	struct VertexPositionColor;
};


class Renderer {
public:
	Renderer(ID3D11Device* device, ID3D11DeviceContext* context);
	~Renderer();

	void Initialize(int width, int height);
	void BeginFrame();
	void EndFrame();

	//	3D•`‰æ
	void SetCamera(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot);
	void DrawGrid();
	void DrawEnemies(const std::vector<Enemy>& enemies);
	void DrawParticles(const std::vector<Particle>& particles);
	void DrawWeapon(WeaponType type, float swayX, float swayY);

	//	UI•`‰æ
	void DrawUI(int health, int wave, int points, int currentAmmo, int reserveAmmo, bool isReloading);
	void DrawDamageFlash(float intensity);

private:
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_context;

	std::unique_ptr<DirectX::CommonStates> m_states;
	std::unique_ptr<DirectX::BasicEffect> m_effect;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
	std::unique_ptr<DirectX::GeometricPrimitive> m_cube;
	std::unique_ptr<DirectX::GeometricPrimitive> m_weaponModel;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	DirectX::XMMATRIX m_viewMatrix;
	DirectX::XMMATRIX m_projectionMatrix;

	int m_width;
	int m_height;

	void DrawSimpleNumber(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch,
						  int dight, float x, float y, DirectX::XMFLOAT4 color);
};