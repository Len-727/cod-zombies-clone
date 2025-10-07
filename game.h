// Game.h - �Q�[���N���X��`�i�\���𗝉����悤�j
#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>          
#include <algorithm>
#include <CommonStates.h>
#include <Effects.h>
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <GeometricPrimitive.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <SimpleMath.h>
#include <SimpleMath.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <WICTextureLoader.h>   // PNG/JPG�Ȃ�
#include <DDSTextureLoader.h>   // DDS�e�N�X�`��
#include <CommonStates.h>
#include "Entities.h"
#include "WeaponSystem.h"

class Game
{
public:
    Game() noexcept;
    ~Game() = default;

    // �R�s�[�֎~�iDirectX���\�[�X�͕����s�j
    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // === �Q�[���̊�{���\�b�h�i�����𗝉����悤�j ===
    void Initialize(HWND window, int width, int height);  // ������
    void Tick();                                          // ���C�����[�v
    void OnWindowSizeChanged(int width, int height);      // �E�B���h�E�T�C�Y�ύX

private:

    std::unique_ptr<WeaponSystem> m_weaponSystem;   //  ����Ǘ��V�X�e��

    // === �Q�[�����[�v�̓������� ===
    void Update();     // �Q�[�����W�b�N�X�V
    void Render();     // �`�揈��


    void Clear();
    void CreateDevice();
    void CreateResources();
    void CreateRenderResources();
    void CreateExplosion(DirectX::XMFLOAT3 position);
    void CreateMuzzleFlash();
    void CreateMuzzleParticles();

    void DrawGrid();
    //void DrawCubes();
    //void DrawHealthBar(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch);
    void DrawBillboard();
    void DrawWeapon();
    void DrawParticles();
    void DrawEnemies();
    void DrawUI();
    void DrawSimpleNumber(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, int digit, float x, float y, DirectX::XMFLOAT4 color);


    void SpawnEnemy();

    void UpdateTitle();
    void UpdatePlaying();
    void UpdateGameOver();
    void UpdateFade();
    void UpdateParticles();
    void UpdateEnemies();

    void RenderTitle();
    void RenderPlaying();
    void RenderGameOver();
    void RenderFade();
    void RenderDamageFlash();



    bool CheckRayHitsKube(DirectX::XMFLOAT3 rayStart, DirectX::XMFLOAT3 rayDir, DirectX::XMFLOAT3 cubePos);

    // DirectX�f�o�C�X�iGPU����j
    Microsoft::WRL::ComPtr<ID3D11Device>            m_d3dDevice;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext>     m_d3dContext;
    Microsoft::WRL::ComPtr<IDXGISwapChain>          m_swapChain;

    // �����_�����O�^�[�Q�b�g�i�`���j
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_renderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView>  m_depthStencilView;

    // �E�B���h�E���
    HWND    m_window;
    int     m_outputWidth;
    int     m_outputHeight;

    // === �Q�[����� ===
    // TODO: �v���C���[�ʒu�A�J�����A�G�L�����Ȃǂ�ǉ�
    DirectX::XMFLOAT3 m_cameraPos;          //�J�����̈ʒu
    DirectX::XMFLOAT3 m_cameraRot;          //�J�����̉�]
    //float             m_mouseSensitivity;   //�}�X�����x
    bool              m_firstMouse;         //����}�E�X�C�ړ��t���O
    int               m_lastMouseX;         //�O�t���[���̃}�E�XX���W
    int               m_lastMouseY;         //�O�t���[���̃}�XY���W

    //  3D�`��p
    std::unique_ptr<DirectX::CommonStates>      m_states;
    std::unique_ptr<DirectX::BasicEffect>       m_effect;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>   m_inputLayout;
    std::unique_ptr<DirectX::GeometricPrimitive> m_cube;


    std::unique_ptr<DirectX::GeometricPrimitive> m_weaponModel;
    //std::unique_ptr<DirectX::GeometricPrimitive> m_muzzleFlashModel;
    float m_weaponSwayX;
    float m_weaponSwayY;
    float m_lastCameraRotX;
    float m_lastCameraRotY;

    
    std::vector<Particle> m_particles;
    bool m_showMuzzleFlash;
    float m_muzzleFlashTimer;

   
    std::vector<Enemy> m_enemies;
    float m_enemySpawnTimer = 0.0f;
    int m_maxEnemies;

    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;



    bool m_mouseClicked;
    bool m_lastMouseState;
    bool m_mouseCaptured;

    //  �L���[�u�j��
    //bool m_cubesDestroyed[3];
    int m_score;
    // �r���{�[�h�\���p
    bool m_showDamageDisplay;
    float m_damageDisplayTimer;
    DirectX::XMFLOAT3 m_damageDisplayPos;
    int m_damageValue;

   

    GameState m_gameState;
    float m_fadeAlpha;
    bool m_fadingIn;
    bool m_fadeActive;


    //  zombies���[�h
    int m_currentWave;
    int m_enemiesPerWave;
    int m_enemiesKilledThisWave;
    int m_totalEnemiesThisWave;
    bool m_betweenWaves;
    float m_waveStartTimer;

    //  �v���C���[
    int m_playerHealth;
    float m_damageTimer;
    bool m_isDamaged;

    //  �|�C���g�V�X�e��
    int m_points;


    //  UI�p
    std::unique_ptr<DirectX::SpriteFont> m_font;
    std::unique_ptr<DirectX::SpriteFont> m_fontLarge;

};
