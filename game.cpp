// Game.cpp - �����i��Օ����j
#include "Game.h"
#include <string>
//#include <stdexcept>
//#include <algorithm>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// DirectXTK�p
#pragma comment(lib, "DirectXTK.lib")


using namespace DirectX;
using Microsoft::WRL::ComPtr;

Game::Game() noexcept :
    m_window(nullptr),
    m_outputWidth(1280),
    m_outputHeight(720),

    //�J�����|�W�V����
    m_cameraPos(0.0f, 0.5f, -0.5f),
    m_cameraRot(0.0f, 0.0f, 0.0f),
    m_firstMouse(true),
    m_lastMouseX(0),
    m_lastMouseY(0),
    m_cube(nullptr),
    m_lastMouseState(false),
    m_mouseCaptured(false),
    m_score(0),
    m_damageDisplayTimer(0.0f),
    m_showDamageDisplay(false),
    m_damageValue(0),
    m_gameState(GameState::TITLE),
    m_fadeAlpha(0.0f),
    m_fadingIn(false),
    m_fadeActive(false),
    m_weaponSwayX(0.0f),
    m_weaponSwayY(0.0f),
    m_lastCameraRotX(0.0f),
    m_lastCameraRotY(0.0f),
    m_showMuzzleFlash(false),
    m_muzzleFlashTimer(0.0f),
    m_maxEnemies(24),
    m_currentWave(1),
    m_enemiesPerWave(6),
    m_enemiesKilledThisWave(0),
    m_totalEnemiesThisWave(6),
    m_betweenWaves(true),
    m_waveStartTimer(5.0f),
    m_playerHealth(100),
    m_damageTimer(0.0f),
    m_isDamaged(false),
    m_points(500),
    m_currentAmmo(30),
    m_maxAmmo(30),
    m_reserveAmmo(90),
    m_isReloading(false),
    m_reloadTimer(0.0f),
    m_currentWeapon(WeaponType::PISTOL),
    m_primaryWeapon(WeaponType::PISTOL),
    m_fireRateTimer(0.0f)
{

    // ����f�[�^�̐ݒ�
    m_weaponStats[WeaponType::PISTOL] = {
        WeaponType::PISTOL, 30, 8, 80, 0.2f, 50.0f, 1, 1.5f, 0
    };
    m_weaponStats[WeaponType::SHOTGUN] = {
        WeaponType::SHOTGUN, 200, 2, 60, 0.8f, 10.0f, 1, 2.5f, 500
    };
    m_weaponStats[WeaponType::RIFLE] = {
        WeaponType::RIFLE, 18, 30, 180, 1.0f, 80, 1, 2.1f, 1000
    };

    // �e����̏����e���Ԃ�ݒ�
    m_weaponAmmoStatus[WeaponType::PISTOL] = { 8, 80 };
    m_weaponAmmoStatus[WeaponType::SHOTGUN] = { 2, 60 };
    m_weaponAmmoStatus[WeaponType::RIFLE] = { 20, 180 };
    m_weaponAmmoStatus[WeaponType::SNIPER] = { 5, 50 };

    // ��������i�s�X�g���j�̒e���ݒ�
    m_currentAmmo = m_weaponAmmoStatus[WeaponType::PISTOL].currentAmmo;
    m_reserveAmmo = m_weaponAmmoStatus[WeaponType::PISTOL].reserveAmmo;

    static bool firstFrame = true;
    if (firstFrame && m_gameState == GameState::PLAYING)
    {
        firstFrame = false;
        m_betweenWaves = true;
        m_waveStartTimer = 3.0f;  // 3�b��ɍŏ��̃E�F�[�u�J�n
    }
}


void Game::Initialize(HWND window, int width, int height)
{
    m_window = window;
    m_outputWidth = (width > 1) ? width : 1;
    m_outputHeight = (height > 1) ? height : 1;

    // DirectX�������i���G�Ȃ̂ŃR�s�yOK�j
    CreateDevice();
    CreateResources();

    CreateRenderResources();  // 3D�`��p�̏�����
}

void Game::Tick()
{
    // �Q�[����1�t���[������
    Update();  // �Q�[�����W�b�N�i�����ŏ��������j
    Render();  // �`�揈��
}

void Game::Update()
{

    switch (m_gameState)
    {
    case GameState::TITLE:
        UpdateTitle();
        break;

    case GameState::PLAYING:
        UpdatePlaying();
        break;

    case GameState::GAMEOVER:
        UpdateGameOver();
        break;
    }

    UpdateFade();





}

void Game::Clear()
{


    m_d3dContext->ClearRenderTargetView(m_renderTargetView.Get(), Colors::Blue);
    // Colors::Red, Colors::Green, Colors::Black �Ȃǂ������Ă݂悤

    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(),
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    m_d3dContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(),
        m_depthStencilView.Get());

    CD3D11_VIEWPORT viewport(0.0f, 0.0f,
        static_cast<float>(m_outputWidth), static_cast<float>(m_outputHeight));
    m_d3dContext->RSSetViewports(1, &viewport);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    m_outputWidth = (width > 1) ? width : 1;
    m_outputHeight = (height > 1) ? height : 1;
    CreateResources();
}


void Game::CreateDevice()
{
    UINT creationFlags = 0;
#ifdef _DEBUG
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;  // �f�o�b�O���L��
#endif

    static const D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,  // DirectX 11.1
        D3D_FEATURE_LEVEL_11_0,  // DirectX 11.0
        D3D_FEATURE_LEVEL_10_1,  // DirectX 10.1�i�t�H�[���o�b�N�j
        D3D_FEATURE_LEVEL_10_0,  // DirectX 10.0�i�t�H�[���o�b�N�j
    };

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // �f�t�H���g�A�_�v�^�[
        D3D_DRIVER_TYPE_HARDWARE,   // �n�[�h�E�F�A�iGPU�j�g�p
        nullptr,
        creationFlags,
        featureLevels,
        4,
        D3D11_SDK_VERSION,
        device.GetAddressOf(),
        nullptr,
        context.GetAddressOf()
    );

    if (FAILED(hr))
        throw std::runtime_error("D3D11CreateDevice failed");

    device.As(&m_d3dDevice);
    context.As(&m_d3dContext);
}

void Game::CreateResources()
{
    // ���\�[�X�N���A
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    m_d3dContext->OMSetRenderTargets(1, nullViews, nullptr);
    m_renderTargetView.Reset();
    m_depthStencilView.Reset();
    m_d3dContext->Flush();

    const UINT backBufferWidth = static_cast<UINT>(m_outputWidth);
    const UINT backBufferHeight = static_cast<UINT>(m_outputHeight);

    if (m_swapChain)
    {
        // �X���b�v�`�F�[�����T�C�Y
        HRESULT hr = m_swapChain->ResizeBuffers(2, backBufferWidth, backBufferHeight,
            DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        if (FAILED(hr)) return;
    }
    else
    {
        // �X���b�v�`�F�[���쐬
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = backBufferWidth;
        swapChainDesc.BufferDesc.Height = backBufferHeight;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = m_window;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = TRUE;

        ComPtr<IDXGIDevice> dxgiDevice;
        m_d3dDevice.As(&dxgiDevice);

        ComPtr<IDXGIAdapter> dxgiAdapter;
        dxgiDevice->GetAdapter(dxgiAdapter.GetAddressOf());

        ComPtr<IDXGIFactory> dxgiFactory;
        dxgiAdapter->GetParent(IID_PPV_ARGS(dxgiFactory.GetAddressOf()));

        HRESULT hr = dxgiFactory->CreateSwapChain(m_d3dDevice.Get(),
            &swapChainDesc, m_swapChain.GetAddressOf());
        if (FAILED(hr))
            throw std::runtime_error("CreateSwapChain failed");
    }

    // �����_�[�^�[�Q�b�g�쐬
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hr)) throw std::runtime_error("GetBuffer failed");

    hr = m_d3dDevice->CreateRenderTargetView(backBuffer.Get(), nullptr,
        m_renderTargetView.GetAddressOf());
    if (FAILED(hr)) throw std::runtime_error("CreateRenderTargetView failed");

    // �[�x�o�b�t�@�쐬
    CD3D11_TEXTURE2D_DESC depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,
        backBufferWidth, backBufferHeight, 1, 1, D3D11_BIND_DEPTH_STENCIL);

    ComPtr<ID3D11Texture2D> depthStencil;
    hr = m_d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, depthStencil.GetAddressOf());
    if (FAILED(hr)) throw std::runtime_error("CreateTexture2D failed");

    hr = m_d3dDevice->CreateDepthStencilView(depthStencil.Get(), nullptr,
        m_depthStencilView.GetAddressOf());
    if (FAILED(hr)) throw std::runtime_error("CreateDepthStencilView failed");
}

void Game::CreateRenderResources()
{
    // DirectXTK�̊�{�I�u�W�F�N�g���쐬
    m_states = std::make_unique<DirectX::CommonStates>(m_d3dDevice.Get());
    m_effect = std::make_unique<DirectX::BasicEffect>(m_d3dDevice.Get());

    // �G�t�F�N�g�̐ݒ�
    m_effect->SetVertexColorEnabled(true);  // ���_�J���[���g�p

    // ���̓��C�A�E�g�̍쐬�iGPU�p�̐ݒ�j
    void const* shaderByteCode;
    size_t byteCodeLength;
    m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);
    HRESULT hr = m_d3dDevice->CreateInputLayout(
        DirectX::VertexPositionColor::InputElements,
        DirectX::VertexPositionColor::InputElementCount,
        shaderByteCode, byteCodeLength,
        m_inputLayout.GetAddressOf()
    );

    if (FAILED(hr))
        throw std::runtime_error("CreateInputLayout failed");

    m_cube = DirectX::GeometricPrimitive::CreateCube(m_d3dContext.Get());

    m_weaponModel = DirectX::GeometricPrimitive::CreateBox(m_d3dContext.Get(),
        DirectX::XMFLOAT3(0.1f, 0.05f, 0.4f));

    m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(m_d3dContext.Get());

    // �t�H���g�t�@�C����ǂݍ���
    //m_font = std::make_unique<DirectX::SpriteFont>(m_d3dDevice.Get(), L"Arial.spritefont");

}

void Game::CreateExplosion(DirectX::XMFLOAT3 position)
{
    // �����G�t�F�N�g�p�p�[�e�B�N������
    for (int i = 0; i < 20; i++)
    {
        Particle particle;
        particle.position = position;

        // �����_���ȕ����ɔ�юU��
        float angle = (float)rand() / RAND_MAX * 2.0f * 3.14159f;
        float speed = 5.0f + (float)rand() / RAND_MAX * 10.0f;

        particle.velocity.x = cosf(angle) * speed;
        particle.velocity.y = 5.0f + (float)rand() / RAND_MAX * 10.0f;
        particle.velocity.z = sinf(angle) * speed;

        particle.color = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

        particle.lifetime = 1.0f + (float)rand() / RAND_MAX * 2.0f;
        particle.maxLifetime = particle.lifetime;

        m_particles.push_back(particle);
    }
}

void Game::CreateMuzzleFlash()
{
    m_showMuzzleFlash = true;
    m_muzzleFlashTimer = 0.05f;

    CreateMuzzleParticles();
}
void Game::CreateMuzzleParticles()
{
    // �e���ʒu���v�Z
    DirectX::XMFLOAT3 muzzlePosition;
    muzzlePosition.x = m_cameraPos.x + 0.45f;
    muzzlePosition.y = m_cameraPos.y - 0.15f;
    muzzlePosition.z = m_cameraPos.z + 0.8f;

    // �Ήԃp�[�e�B�N�������i�����ŒZ���ԁj
    // ���M�����Ήԁi�����̋����Ёj
    for (int i = 0; i < 6; i++)
    {
        Particle particle;
        particle.position = muzzlePosition;

        float spreadAngle = ((float)rand() / RAND_MAX - 0.5f) * 0.8f;
        float speed = 20.0f + (float)rand() / RAND_MAX * 15.0f;

        particle.velocity.x = sinf(m_cameraRot.y + spreadAngle) * speed;
        particle.velocity.y = -sinf(m_cameraRot.x) * speed + ((float)rand() / RAND_MAX - 0.5f) * 3.0f;
        particle.velocity.z = cosf(m_cameraRot.y + spreadAngle) * speed;

        // ���`�����F�i�����̋����j
        particle.color = DirectX::XMFLOAT4(1.0f, 1.0f, 0.9f + (float)rand() / RAND_MAX * 0.1f, 1.0f);

        particle.lifetime = 0.15f + (float)rand() / RAND_MAX * 0.1f;
        particle.maxLifetime = particle.lifetime;

        m_particles.push_back(particle);
    }

    // �R�ăK�X�i�I�����W�F�j
    for (int i = 0; i < 4; i++)
    {
        Particle particle;
        particle.position = muzzlePosition;

        float spreadAngle = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
        float speed = 8.0f + (float)rand() / RAND_MAX * 8.0f;

        particle.velocity.x = sinf(m_cameraRot.y + spreadAngle) * speed;
        particle.velocity.y = -sinf(m_cameraRot.x) * speed + ((float)rand() / RAND_MAX) * 5.0f;
        particle.velocity.z = cosf(m_cameraRot.y + spreadAngle) * speed;

        // �I�����W�`�ԐF�i�R�ăK�X�j
        particle.color = DirectX::XMFLOAT4(1.0f, 0.4f + (float)rand() / RAND_MAX * 0.4f, 0.1f, 1.0f);

        particle.lifetime = 0.3f + (float)rand() / RAND_MAX * 0.2f;
        particle.maxLifetime = particle.lifetime;

        m_particles.push_back(particle);
    }
}

void Game::UpdateEnemies()
{
    // �f�o�b�O�o��
    char debug[256];
    sprintf_s(debug, "Wave starting in: %.1f\n", m_waveStartTimer);
    OutputDebugStringA(debug);

    float deltaTime = 1.0f / 60.0f; // 60FPS�z��

    // �G�̍X�V
    for (auto& enemy : m_enemies)
    {
        if (!enemy.isAlive)
            continue;

        // �ړ��^�C�}�[�X�V
        enemy.moveTimer += deltaTime;

        // �����ύX�̃^�C�~���O
        if (enemy.moveTimer >= enemy.nextDirectionChange)
        {
            // �v���C���[�Ɍ������������v�Z
            float dirX = m_cameraPos.x - enemy.position.x;
            float dirZ = m_cameraPos.z - enemy.position.z;
            float distance = sqrtf(dirX * dirX + dirZ * dirZ);

            if (distance > 0.1f)
            {
                // ���K�����ăv���C���[�����Ɉړ�
                enemy.velocity.x = (dirX / distance) * 3.0f;
                enemy.velocity.z = (dirZ / distance) * 3.0f;
            }

            // ���̕����ύX�܂ł̎��Ԃ����Z�b�g
            enemy.moveTimer = 0.0f;
            enemy.nextDirectionChange = 1.0f + (float)rand() / RAND_MAX * 2.0f;
        }

        // �ʒu�X�V
        enemy.position.x += enemy.velocity.x * deltaTime;
        enemy.position.z += enemy.velocity.z * deltaTime;

        // �n�ʂ̍������ێ�
        enemy.position.y = 1.0f;

        // �}�b�v���E�`�F�b�N�i�ȈՔŁj
        if (enemy.position.x < -50.0f || enemy.position.x > 50.0f ||
            enemy.position.z < -50.0f || enemy.position.z > 50.0f)
        {
            // �͈͊O�ɏo���璆�����ɖ߂�
            enemy.velocity.x *= -0.5f;
            enemy.velocity.z *= -0.5f;
        }

        // �v���C���[�Ƃ̐ڐG����
        float playerDist = sqrtf(
            powf(enemy.position.x - m_cameraPos.x, 2) +
            powf(enemy.position.z - m_cameraPos.z, 2)
        );

        if (playerDist < 1.5f && m_damageTimer <= 0.0f)
        {
            m_playerHealth -= 10;
            m_damageTimer = 1.0f;  // 1�b�̖��G����
            m_isDamaged = true;

            if (m_playerHealth <= 0)
            {
                m_gameState = GameState::GAMEOVER;
            }
        }
    }

    // �E�F�[�u�Ǘ�
    if (m_betweenWaves)
    {
        m_waveStartTimer -= deltaTime;
        if (m_waveStartTimer <= 0.0f)
        {
            // �V�E�F�[�u�J�n
            m_betweenWaves = false;
            m_enemiesKilledThisWave = 0;
            m_totalEnemiesThisWave = m_enemiesPerWave;

            // �����X�|�[��
            int initialSpawn = min(2 + (m_currentWave - 1), 4);
            initialSpawn = min(initialSpawn, m_totalEnemiesThisWave);
            for (int i = 0; i < initialSpawn; i++)
            {
                SpawnEnemy();
            }
        }
    }
    else
    {
        // �E�F�[�u���̃X�|�[���Ǘ�
        int currentEnemyCount = 0;
        for (const auto& enemy : m_enemies)
        {
            if (enemy.isAlive) currentEnemyCount++;
        }

        int totalSpawned = m_enemiesKilledThisWave + currentEnemyCount;
        if (totalSpawned < m_totalEnemiesThisWave && currentEnemyCount < 24)
        {
            m_enemySpawnTimer += deltaTime;

            // �E�F�[�u���i�ނɂ�ăX�|�[�����x�A�b�v
            float spawnInterval = 2.0f - (m_currentWave * 0.1f);
            spawnInterval = max(0.5f, spawnInterval);

            if (m_enemySpawnTimer >= spawnInterval)
            {
                SpawnEnemy();
                m_enemySpawnTimer = 0.0f;
            }
        }

        // �E�F�[�u�N���A����
        if (m_enemiesKilledThisWave >= m_totalEnemiesThisWave)
        {
            m_currentWave++;
            m_enemiesPerWave = 6 + (m_currentWave - 1) * 3;  // ���E�F�[�u3�̑���
            m_betweenWaves = true;
            m_waveStartTimer = 10.0f;  // 10�b�̏�������
            m_points += 100;  // �E�F�[�u�N���A�{�[�i�X
        }
    }

    // ���񂾓G���폜
    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const Enemy& e) { return !e.isAlive; }),
        m_enemies.end()
    );


    //// �G�̃X�|�[���Ǘ�
    //m_enemySpawnTimer += deltaTime;
    //if (m_enemySpawnTimer >= 5.0f) // 5�b���ƂɐV�����G���X�|�[��
    //{
    //    SpawnEnemy();
    //    m_enemySpawnTimer = 0.0f;
    //}

    //// �����Ă���G�̐����J�E���g
    //int aliveEnemies = 0;
    //for (const auto& enemy : m_enemies)
    //{
    //    if (enemy.isAlive)
    //        aliveEnemies++;
    //}

    //// �G�����Ȃ��Ȃ������[
    //if (aliveEnemies < 2)
    //{
    //    SpawnEnemy();
    //}

    //// ���񂾓G��z�񂩂�폜
    //m_enemies.erase(
    //    std::remove_if(m_enemies.begin(), m_enemies.end(),
    //        [](const Enemy& e) { return !e.isAlive; }),
    //    m_enemies.end()
    //);

}


void Game::DrawGrid()
{
    // 3D�`��̏���
    auto context = m_d3dContext.Get();

    // �r���[�s��i�J�����ʒu�E�����j�̍쐬
    DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_cameraPos);

    // �J�����������Ă���������v�Z
    DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(
        m_cameraPos.x + sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
        m_cameraPos.y - sinf(m_cameraRot.x),
        m_cameraPos.z + cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
        0.0f
    );

    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraTarget, upVector);

    // �v���W�F�N�V�����s��i�������e�j
    float aspectRatio = (float)m_outputWidth / (float)m_outputHeight;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(70.0f), aspectRatio, 0.1f, 1000.0f
    );

    // �G�t�F�N�g�ɍs���ݒ�
    m_effect->SetView(viewMatrix);
    m_effect->SetProjection(projectionMatrix);
    m_effect->SetWorld(DirectX::XMMatrixIdentity());

    // �`��J�n
    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());

    // �v���~�e�B�u�o�b�`�Ő��`��
    auto primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
    primitiveBatch->Begin();

    // �O���b�h���̕`��
    DirectX::XMFLOAT4 gridColor(0.5f, 0.5f, 0.5f, 1.0f); // �O���[

    // Z�����̐��i��O���牜�ցj
    for (int x = -10; x <= 10; x += 2)
    {
        primitiveBatch->DrawLine(
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(x, 0, -10), gridColor),
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(x, 0, 10), gridColor)
        );
    }

    // X�����̐��i������E�ցj
    for (int z = -10; z <= 10; z += 2)
    {
        primitiveBatch->DrawLine(
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(-10, 0, z), gridColor),
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(10, 0, z), gridColor)
        );
    }

    primitiveBatch->End();
}

//void Game::DrawCubes()
//{
//    // �r���[�E�v���W�F�N�V�����s���DrawGrid�Ɠ������̂��g�p
//    DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_cameraPos);
//    DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(
//        m_cameraPos.x + sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
//        m_cameraPos.y - sinf(m_cameraRot.x),
//        m_cameraPos.z + cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
//        0.0f
//    );
//    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
//    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraTarget, upVector);
//
//    float aspectRatio = (float)m_outputWidth / (float)m_outputHeight;
//    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
//        DirectX::XMConvertToRadians(70.0f), aspectRatio, 0.1f, 1000.0f
//    );
//
//
//    // �L���[�u1�F�ԐF(�j�󂳂�Ă��Ȃ���Ε`��)
//    if (!m_cubesDestroyed[0])
//    {
//        DirectX::XMMATRIX world1 = DirectX::XMMatrixTranslation(5.0f, 1.0f, 5.0f);
//        m_cube->Draw(world1, viewMatrix, projectionMatrix, DirectX::Colors::Red);
//    }
//
//    if (!m_cubesDestroyed[1])
//    {
//        // �L���[�u2�F�F
//        DirectX::XMMATRIX world2 = DirectX::XMMatrixTranslation(-3.0f, 1.0f, 8.0f);
//        m_cube->Draw(world2, viewMatrix, projectionMatrix, DirectX::Colors::Blue);
//    }
//
//    if (!m_cubesDestroyed[2])
//    {
//        // �L���[�u3�F�ΐF
//        DirectX::XMMATRIX world3 = DirectX::XMMatrixTranslation(0.0f, 1.0f, 15.0f);
//        m_cube->Draw(world3, viewMatrix, projectionMatrix, DirectX::Colors::Green);
//    }
//}

void Game::DrawEnemies()
{
    // �r���[�E�v���W�F�N�V�����s��̌v�Z
    DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_cameraPos);
    DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(
        m_cameraPos.x + sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
        m_cameraPos.y - sinf(m_cameraRot.x),
        m_cameraPos.z + cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
        0.0f
    );
    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraTarget, upVector);

    float aspectRatio = (float)m_outputWidth / (float)m_outputHeight;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(70.0f), aspectRatio, 0.1f, 1000.0f
    );

    // �G��`��
    for (const auto& enemy : m_enemies)
    {
        if (!enemy.isAlive)
            continue;

        DirectX::XMMATRIX world = DirectX::XMMatrixTranslation(
            enemy.position.x, enemy.position.y, enemy.position.z
        );

        DirectX::XMVECTOR color = DirectX::XMLoadFloat4(&enemy.color);
        m_cube->Draw(world, viewMatrix, projectionMatrix, color);
    }

    // HP�o�[��`��
    auto context = m_d3dContext.Get();
    m_effect->SetView(viewMatrix);
    m_effect->SetProjection(projectionMatrix);
    m_effect->SetWorld(DirectX::XMMatrixIdentity());
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetDiffuseColor(DirectX::Colors::White);
    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());

    auto primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
    primitiveBatch->Begin();

    for (const auto& enemy : m_enemies)
    {
        if (!enemy.isAlive || enemy.health >= enemy.maxHealth)
            continue;

        // HP�o�[�̈ʒu�i�G�̏�j
        float barWidth = 1.0f;
        float barHeight = 0.1f;
        float healthPercent = (float)enemy.health / enemy.maxHealth;

        DirectX::XMFLOAT3 barCenter = enemy.position;
        barCenter.y += 2.5f;

        // �w�i�i�ԁj
        DirectX::XMFLOAT4 bgColor(0.5f, 0.0f, 0.0f, 1.0f);
        primitiveBatch->DrawLine(
            DirectX::VertexPositionColor(
                DirectX::XMFLOAT3(barCenter.x - barWidth / 2, barCenter.y, barCenter.z),
                bgColor
            ),
            DirectX::VertexPositionColor(
                DirectX::XMFLOAT3(barCenter.x + barWidth / 2, barCenter.y, barCenter.z),
                bgColor
            )
        );

        // HP�����i���邢�ԁj
        DirectX::XMFLOAT4 hpColor(1.0f, 0.0f, 0.0f, 1.0f);
        float currentBarWidth = barWidth * healthPercent;
        primitiveBatch->DrawLine(
            DirectX::VertexPositionColor(
                DirectX::XMFLOAT3(barCenter.x - barWidth / 2, barCenter.y + 0.05f, barCenter.z),
                hpColor
            ),
            DirectX::VertexPositionColor(
                DirectX::XMFLOAT3(barCenter.x - barWidth / 2 + currentBarWidth, barCenter.y + 0.05f, barCenter.z),
                hpColor
            )
        );
    }

    primitiveBatch->End();
}

bool Game::CheckRayHitsKube(DirectX::XMFLOAT3 rayStart, DirectX::XMFLOAT3 rayDir, DirectX::XMFLOAT3 cubePos)
{
    // ���C�i�����j�ƃL���[�u�̓����蔻��
    // �L���[�u�̃T�C�Y�i1x1x1�Ɖ���j
    float cubeSize = 2.0f;

    // �L���[�u�̋��E���v�Z
    float minX = cubePos.x - cubeSize * 0.5f;
    float maxX = cubePos.x + cubeSize * 0.5f;
    float minY = cubePos.y - cubeSize * 0.5f;
    float maxY = cubePos.y + cubeSize * 0.5f;
    float minZ = cubePos.z - cubeSize * 0.5f;
    float maxZ = cubePos.z + cubeSize * 0.5f;

    // ���C�ƃ{�b�N�X�̌�_�v�Z
    float tMin = (minX - rayStart.x) / rayDir.x;
    float tMax = (maxX - rayStart.x) / rayDir.x;

    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (minY - rayStart.y) / rayDir.y;
    float tyMax = (maxY - rayStart.y) / rayDir.y;

    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if (tMin > tyMax || tyMin > tMax) return false;

    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    float tzMin = (minZ - rayStart.z) / rayDir.z;
    float tzMax = (maxZ - rayStart.z) / rayDir.z;

    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if (tMin > tzMax || tzMin > tMax) return false;

    return tMin > 0; // ���C���O�����ɐi��ł��邱�Ƃ��m�F
}



void Game::DrawBillboard()
{
    if (!m_showDamageDisplay)
        return;

    // ������DrawCubes�Ɠ����`��ݒ�
    DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_cameraPos);
    DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(
        m_cameraPos.x + sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
        m_cameraPos.y - sinf(m_cameraRot.x),
        m_cameraPos.z + cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
        0.0f
    );
    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraTarget, upVector);

    float aspectRatio = (float)m_outputWidth / (float)m_outputHeight;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(70.0f), aspectRatio, 0.1f, 1000.0f
    );

    // �����ȉ��F���L���[�u���_���[�W�\���ʒu�ɕ`��
    DirectX::XMMATRIX world = DirectX::XMMatrixScaling(0.3f, 0.3f, 0.3f) *  // ����������
        DirectX::XMMatrixTranslation(m_damageDisplayPos.x,
            m_damageDisplayPos.y,
            m_damageDisplayPos.z);

    m_cube->Draw(world, viewMatrix, projectionMatrix, DirectX::Colors::Yellow);
}

void Game::DrawParticles()
{
    if (m_particles.empty())
        return;

    auto context = m_d3dContext.Get();

    // �r���[�E�v���W�F�N�V�����s��̌v�Z
    DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_cameraPos);
    DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(
        m_cameraPos.x + sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
        m_cameraPos.y - sinf(m_cameraRot.x),
        m_cameraPos.z + cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
        0.0f
    );
    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraTarget, upVector);

    float aspectRatio = (float)m_outputWidth / (float)m_outputHeight;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(70.0f), aspectRatio, 0.1f, 1000.0f
    );

    // �d�v�F���_�J���[��L����
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetDiffuseColor(DirectX::Colors::White);
    m_effect->SetView(viewMatrix);
    m_effect->SetProjection(projectionMatrix);
    m_effect->SetWorld(DirectX::XMMatrixIdentity());

    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());

    auto primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
    primitiveBatch->Begin();

    for (const auto& particle : m_particles)
    {
        float size = 0.1f; // �T�C�Y��傫�����Č��₷��

        // ��茩�₷���\���`�ŕ`��
        DirectX::XMFLOAT3 center = particle.position;

        // ����
        primitiveBatch->DrawLine(
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(center.x - size, center.y, center.z), particle.color),
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(center.x + size, center.y, center.z), particle.color)
        );

        // �c��
        primitiveBatch->DrawLine(
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(center.x, center.y - size, center.z), particle.color),
            DirectX::VertexPositionColor(DirectX::XMFLOAT3(center.x, center.y + size, center.z), particle.color)
        );
    }

    primitiveBatch->End();
}


void Game::UpdateTitle()
{
    // �f�o�b�O�F�t�F�[�h�l��\��
    char debug[256];
    sprintf_s(debug, "TITLE - Press SPACE - Fade:%.2f", m_fadeAlpha);
    SetWindowTextA(m_window, debug);

    // �^�C�g����ʁFSpace�L�[�ŃQ�[���J�n
    if (GetAsyncKeyState(VK_SPACE) & 0x8000)
    {
        m_gameState = GameState::PLAYING;
        m_fadeAlpha = 1.0f;
        m_fadingIn = true;
        m_fadeActive = true;
    }


}

void Game::DrawWeapon()
{
    // �J�������΍��W�ŕ����z�u�iHUD�I�ȕ\���j
    DirectX::XMVECTOR cameraPosition = DirectX::XMLoadFloat3(&m_cameraPos);
    DirectX::XMVECTOR cameraTarget = DirectX::XMVectorSet(
        m_cameraPos.x + sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
        m_cameraPos.y - sinf(m_cameraRot.x),
        m_cameraPos.z + cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
        0.0f
    );
    DirectX::XMVECTOR upVector = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixLookAtLH(cameraPosition, cameraTarget, upVector);

    float aspectRatio = (float)m_outputWidth / (float)m_outputHeight;
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        DirectX::XMConvertToRadians(70.0f), aspectRatio, 0.1f, 1000.0f
    );

    // �J�����̑O���E�E���E����x�N�g�����v�Z
    DirectX::XMVECTOR forward = DirectX::XMVectorSet(
        sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
        -sinf(m_cameraRot.x),
        cosf(m_cameraRot.y) * cosf(m_cameraRot.x),
        0.0f
    );

    DirectX::XMVECTOR right = DirectX::XMVectorSet(
        cosf(m_cameraRot.y),
        0.0f,
        -sinf(m_cameraRot.y),
        0.0f
    );

    DirectX::XMVECTOR up = DirectX::XMVector3Cross(forward, right);

    // �J��������̑��Έʒu�ŕ���ʒu���v�Z
    DirectX::XMVECTOR weaponPos = cameraPosition +
        right * (0.3f + m_weaponSwayX * 0.1f) +      // �E��0.3
        up * (-0.2f + m_weaponSwayY * 0.1f) +      // ����0.2
        forward * 0.4f;     // �O��0.4

    // ������J�����̌����ɍ��킹�ĉ�]
    DirectX::XMMATRIX weaponWorld = DirectX::XMMatrixRotationRollPitchYaw(
        m_cameraRot.x, m_cameraRot.y, 0.0f) *
        DirectX::XMMatrixTranslationFromVector(weaponPos);

    m_weaponModel->Draw(weaponWorld, viewMatrix, projectionMatrix, DirectX::Colors::Black);


    //// �e���t���b�V���̕`��
    //if (m_showMuzzleFlash)
    //{
    //    // ����̐�[�ʒu���v�Z
    //    DirectX::XMVECTOR muzzlePos = cameraPosition +
    //        right * (0.35f + m_weaponSwayX * 0.1f) +
    //        up * (-0.15f + m_weaponSwayY * 0.1f) +
    //        forward * 0.8f;

    //    DirectX::XMMATRIX flashWorld = DirectX::XMMatrixTranslationFromVector(muzzlePos);

    //    m_muzzleFlashModel->Draw(flashWorld, viewMatrix, projectionMatrix, DirectX::Colors::Yellow);
    //}


}




// =================================================================
// �y�ŏ�ʁz�`��S�̂̎i�ߓ�
// =================================================================
void Game::Render()
{
    Clear();

    switch (m_gameState)
    {
    case GameState::TITLE:
        RenderTitle();
        break;
    case GameState::PLAYING:
        RenderPlaying();
        break;
    case GameState::GAMEOVER:
        RenderGameOver();
        break;
    }

    RenderFade();
    m_swapChain->Present(1, 0);
}

// =================================================================
// �y�Q�[�����z�̕`�揈��
// =================================================================
void Game::RenderPlaying()
{
    // �ŏ���3D��Ԃ̂��̂����ׂĕ`�悷��
    DrawParticles();
    DrawGrid();
    DrawEnemies();
    DrawBillboard();
    DrawWeapon();

    // �Ō��UI�����ׂĎ�O�ɕ`�悷��
    DrawUI();

    // �_���[�W�G�t�F�N�g����ԏ�ɏd�˂�
    if (m_isDamaged)
    {
        RenderDamageFlash();
    }
}

// =================================================================
// �yUI�z���ׂĂ�UI��`�悷��i�ߓ�
// =================================================================
void Game::DrawUI()
{
    // --- UI�`��̂��߂̋��ʐݒ� ---
    auto context = m_d3dContext.Get();
    m_d3dContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, (float)m_outputWidth, (float)m_outputHeight, 0.0f, 0.1f, 1.0f);

    m_effect->SetProjection(projection);
    m_effect->SetView(DirectX::XMMatrixIdentity());
    m_effect->SetWorld(DirectX::XMMatrixIdentity());
    m_effect->SetVertexColorEnabled(true);
    m_effect->SetDiffuseColor(DirectX::Colors::White);
    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());

    auto primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
    primitiveBatch->Begin();

    // --- ��������eUI�v�f�̕`�揈�� ---

    // (1) �̗̓o�[�̕`�� (����)
    {
        float barWidth = 200.0f, barHeight = 20.0f, padding = 50.0f;
        float startX = padding, startY = m_outputHeight - padding - barHeight;
        DirectX::XMFLOAT4 bgColor(0.2f, 0.2f, 0.2f, 0.8f);
        for (float i = 0; i < barHeight; ++i) {
            primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX, startY + i, 1.0f), bgColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX + barWidth, startY + i, 1.0f), bgColor));
        }
        float healthPercent = (float)m_playerHealth / 100.0f;
        float currentBarWidth = barWidth * healthPercent;
        if (currentBarWidth > 0) {
            DirectX::XMFLOAT4 healthColor;
            if (healthPercent > 0.6f) healthColor = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
            else if (healthPercent > 0.3f) healthColor = DirectX::XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
            else healthColor = DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
            for (float i = 0; i < barHeight; ++i) {
                primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX, startY + i, 1.0f), healthColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX + currentBarWidth, startY + i, 1.0f), healthColor));
            }
        }
        DirectX::XMFLOAT4 borderColor(1.0f, 1.0f, 1.0f, 1.0f);
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX, startY, 1.0f), borderColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX + barWidth, startY, 1.0f), borderColor));
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX, startY + barHeight, 1.0f), borderColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX + barWidth, startY + barHeight, 1.0f), borderColor));
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX, startY, 1.0f), borderColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX, startY + barHeight, 1.0f), borderColor));
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX + barWidth, startY, 1.0f), borderColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(startX + barWidth, startY + barHeight, 1.0f), borderColor));
    }

    // (2) �N���X�w�A�̕`�� (����)
    {
        DirectX::XMFLOAT4 crosshairColor(1.0f, 1.0f, 1.0f, 1.0f);
        float centerX = m_outputWidth / 2.0f, centerY = m_outputHeight / 2.0f, size = 20.0f;
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(centerX, centerY - size, 1.0f), crosshairColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(centerX, centerY + size, 1.0f), crosshairColor));
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(centerX - size, centerY, 1.0f), crosshairColor), DirectX::VertexPositionColor(DirectX::XMFLOAT3(centerX + size, centerY, 1.0f), crosshairColor));
    }

    // (3) �E�F�[�u���̕`�� (�㒆��)
    {
        DirectX::XMFLOAT4 color(1.0f, 1.0f, 0.0f, 1.0f); float digitWidth = 15.0f; float digitSpacing = 20.0f;
        int wave = m_currentWave;
        if (wave == 0) { DrawSimpleNumber(primitiveBatch.get(), 0, (m_outputWidth - digitWidth) / 2.0f, 50.0f, color); }
        else {
            std::vector<int> digits; while (wave > 0) { digits.push_back(wave % 10); wave /= 10; }
            int numDigits = digits.size(); float totalWidth = numDigits * digitWidth + (numDigits - 1) * (digitSpacing - digitWidth);
            float startX = (m_outputWidth - totalWidth) / 2.0f; float startY = 50.0f;
            for (int i = 0; i < numDigits; ++i) { DrawSimpleNumber(primitiveBatch.get(), digits[numDigits - 1 - i], startX + i * digitSpacing, startY, color); }
        }
    }

    // (4) �|�C���g�̕`�� (�E��)
    {
        DirectX::XMFLOAT4 color(0.1f, 1.0f, 1.0f, 1.0f); float digitWidth = 15.0f; float digitSpacing = 20.0f; float padding = 50.0f;
        int points = m_points;
        if (points == 0) { DrawSimpleNumber(primitiveBatch.get(), 0, m_outputWidth - padding - digitWidth, padding, color); }
        else {
            std::vector<int> digits; while (points > 0) { digits.push_back(points % 10); points /= 10; }
            int numDigits = digits.size(); float totalWidth = numDigits * digitWidth + (numDigits - 1) * (digitSpacing - digitWidth);
            float startX = m_outputWidth - padding - totalWidth; float startY = padding;
            for (int i = 0; i < numDigits; ++i) { DrawSimpleNumber(primitiveBatch.get(), digits[numDigits - 1 - i], startX + i * digitSpacing, startY, color); }
        }
    }

    // (5) �e��̕`�� (�E��)
    {
        DirectX::XMFLOAT4 color(1.0f, 1.0f, 1.0f, 1.0f); DirectX::XMFLOAT4 reloadingColor(1.0f, 0.2f, 0.2f, 1.0f);
        float digitHeight = 25.0f; float digitWidth = 15.0f; float digitSpacing = 20.0f; float separatorWidth = 20.0f; float padding = 50.0f;
        DirectX::XMFLOAT4 currentColor = m_isReloading ? reloadingColor : color;
        std::string currentAmmoStr = std::to_string(m_currentAmmo); std::string reserveAmmoStr = std::to_string(m_reserveAmmo);
        float currentWidth = currentAmmoStr.length() * digitWidth + (currentAmmoStr.length() - 1) * (digitSpacing - digitWidth);
        float reserveWidth = reserveAmmoStr.length() * digitWidth + (reserveAmmoStr.length() - 1) * (digitSpacing - digitWidth);
        float totalWidth = currentWidth + separatorWidth + reserveWidth;
        float startX = m_outputWidth - padding - totalWidth; float startY = m_outputHeight - padding - digitHeight;
        float currentX = startX;
        for (char c : currentAmmoStr) { DrawSimpleNumber(primitiveBatch.get(), c - '0', currentX, startY, currentColor); currentX += digitSpacing; }
        currentX += (separatorWidth - digitSpacing) / 2;
        primitiveBatch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(currentX, startY + digitHeight, 1.0f), color), DirectX::VertexPositionColor(DirectX::XMFLOAT3(currentX + 10.0f, startY, 1.0f), color));
        currentX += separatorWidth - (separatorWidth - digitSpacing) / 2;
        for (char c : reserveAmmoStr) { DrawSimpleNumber(primitiveBatch.get(), c - '0', currentX, startY, color); currentX += digitSpacing; }
    }

    // (6) ���݂̕��햼�\���i�������j
    {
        DirectX::XMFLOAT4 weaponColor(1.0f, 1.0f, 1.0f, 1.0f);
        float centerX = m_outputWidth / 2.0f;
        float bottomY = m_outputHeight - 120.0f;

        // ���햼���V���v���ɕ\���iW=Weapon, 1-4�̐����ŕ\���j
        int weaponNum = (int)m_currentWeapon + 1;
        DrawSimpleNumber(primitiveBatch.get(), weaponNum, centerX - 30, bottomY, weaponColor);
    }

    primitiveBatch->End();
}

// =================================================================
// �y���i�z������`�悷�邽�߂̃w���p�[�֐�
// =================================================================
void Game::DrawSimpleNumber(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch, int digit, float x, float y, DirectX::XMFLOAT4 color) {
    float w = 15.0f; float h = 25.0f;
    auto DrawThickLine = [&](float x1, float y1, float x2, float y2) { batch->DrawLine(DirectX::VertexPositionColor(DirectX::XMFLOAT3(x1, y1, 1.0f), color), DirectX::VertexPositionColor(DirectX::XMFLOAT3(x2, y2, 1.0f), color)); };
    switch (digit) {
    case 0: DrawThickLine(x, y, x + w, y); DrawThickLine(x, y, x, y + h); DrawThickLine(x + w, y, x + w, y + h); DrawThickLine(x, y + h, x + w, y + h); break;
    case 1: DrawThickLine(x + w, y, x + w, y + h); break;
    case 2: DrawThickLine(x, y, x + w, y); DrawThickLine(x + w, y, x + w, y + h / 2); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x, y + h / 2, x, y + h); DrawThickLine(x, y + h, x + w, y + h); break;
    case 3: DrawThickLine(x, y, x + w, y); DrawThickLine(x + w, y, x + w, y + h); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x, y + h, x + w, y + h); break;
    case 4: DrawThickLine(x, y, x, y + h / 2); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x + w, y, x + w, y + h); break;
    case 5: DrawThickLine(x, y, x + w, y); DrawThickLine(x, y, x, y + h / 2); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x + w, y + h / 2, x + w, y + h); DrawThickLine(x, y + h, x + w, y + h); break;
    case 6: DrawThickLine(x, y, x + w, y); DrawThickLine(x, y, x, y + h); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x + w, y + h / 2, x + w, y + h); DrawThickLine(x, y + h, x + w, y + h); break;
    case 7: DrawThickLine(x, y, x + w, y); DrawThickLine(x + w, y, x + w, y + h); break;
    case 8: DrawThickLine(x, y, x + w, y); DrawThickLine(x, y, x, y + h); DrawThickLine(x + w, y, x + w, y + h); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x, y + h, x + w, y + h); break;
    case 9: DrawThickLine(x, y, x + w, y); DrawThickLine(x, y, x, y + h / 2); DrawThickLine(x + w, y, x + w, y + h); DrawThickLine(x, y + h / 2, x + w, y + h / 2); DrawThickLine(x, y + h, x + w, y + h); break;
    }
}
void Game::SwitchWeapon(WeaponType newWeapon)
{
    // ���݂̕���̒e���ۑ�
    m_weaponAmmoStatus[m_currentWeapon] = { m_currentAmmo, m_reserveAmmo };

    // �V��������ɐ؂�ւ�
    m_currentWeapon = newWeapon;
    auto& weapon = m_weaponStats[newWeapon];

    // �ۑ�����Ă���e��𕜌�
    m_currentAmmo = m_weaponAmmoStatus[newWeapon].currentAmmo;
    m_reserveAmmo = m_weaponAmmoStatus[newWeapon].reserveAmmo;
    m_maxAmmo = weapon.maxAmmo;

    // �^�C�}�[�Ȃǂ����Z�b�g
    m_reloadTimer = 0.0f;
    m_isReloading = false;
}


void Game::BuyWeapon(WeaponType weaponType)
{
    // �������Ă��镐��Ɠ����Ȃ�A�e��𖞃^���ɂ���i�e��w���j
    if (m_currentWeapon == weaponType)
    {
        if (m_points >= m_weaponStats[weaponType].cost / 2) // �e��͔��z
        {
            m_points -= m_weaponStats[weaponType].cost / 2;
            m_reserveAmmo = m_weaponStats[weaponType].reserveAmmo;
            m_weaponAmmoStatus[m_currentWeapon].reserveAmmo = m_reserveAmmo;
        }
        return;
    }

    // ���ɏ������Ă��镐��Ȃ�A����ɐ؂�ւ��邾��
    if (m_primaryWeapon == weaponType || (m_hasSecondaryWeapon && m_secondaryWeapon == weaponType))
    {
        SwitchWeapon(weaponType);
        return;
    }

    // --- �V�K�w������ ---
    int cost = m_weaponStats[weaponType].cost;
    if (m_points < cost)
        return;

    m_points -= cost;

    // ���݂̃X���b�g�ɐV����������㏑��
    if (m_currentWeaponSlot == 0) // �v���C�}���X���b�g
    {
        m_primaryWeapon = weaponType;
    }
    else // �Z�J���_���X���b�g
    {
        m_secondaryWeapon = weaponType;
    }

    // �Z�J���_����������߂Ď�ɓ��ꂽ���̏���
    if (!m_hasSecondaryWeapon)
    {
        m_secondaryWeapon = WeaponType::PISTOL; // �Â��v���C�}�����Z�J���_����
        m_hasSecondaryWeapon = true;
    }

    SwitchWeapon(weaponType);
}

void Game::UpdatePlaying()
{
    char debug[256];
    sprintf_s(debug, "Wave:%d | Points:%d | Health:%d | Kills:%d/%d",
        m_currentWave, m_points, m_playerHealth,
        m_enemiesKilledThisWave, m_totalEnemiesThisWave);
    SetWindowTextA(m_window, debug);

    //  �O��ړ�
    if (GetAsyncKeyState('W') & 0x8000)
    {
        float forwardX = sinf(m_cameraRot.y);   //  X��������
        float forwardZ = cosf(m_cameraRot.y);   //  Z��������

        float moveSpeed = 0.1f;
        m_cameraPos.x += forwardX * moveSpeed;
        m_cameraPos.z += forwardZ * moveSpeed;
    }

    if (GetAsyncKeyState('S') & 0x8000)
    {
        float forwardX = sinf(m_cameraRot.y);
        float forwardZ = cosf(m_cameraRot.y);

        float moveSpeed = 0.1f;
        m_cameraPos.x -= forwardX * moveSpeed;
        m_cameraPos.z -= forwardZ * moveSpeed;
    }


    //  ���E�ړ�
    if (GetAsyncKeyState('A') & 0x8000)
    {
        float leftX = sinf(m_cameraRot.y - 1.57f);
        float leftZ = cosf(m_cameraRot.y - 1.57f);

        float moveSpeed = 0.1f;
        m_cameraPos.x += leftX * moveSpeed;
        m_cameraPos.z += leftZ * moveSpeed;
    }

    if (GetAsyncKeyState('D') & 0x8000)
    {
        float rightX = sinf(m_cameraRot.y + 1.57f);
        float rightZ = cosf(m_cameraRot.y + 1.57f);

        float moveSpeed = 0.1f;
        m_cameraPos.x += rightX * moveSpeed;
        m_cameraPos.z += rightZ * moveSpeed;
    }

    //  �}�E�X�Œ萧��
    if (GetAsyncKeyState(VK_TAB) & 0x8000)
    {
        static bool tabPressed = false;
        if (!tabPressed)
        {
            m_mouseCaptured = !m_mouseCaptured;
            if (m_mouseCaptured)
            {
                ShowCursor(FALSE);  //  �J�[�\����\��

                //  �E�B���h�E�����Ƀ}�X���ړ�
                RECT rect;
                GetClientRect(m_window, &rect);
                POINT center = { rect.right / 2, rect.bottom / 2 };
                ClientToScreen(m_window, &center);
                SetCursorPos(center.x, center.y);

                SetWindowTextA(m_window, "�}�E�X�Œ�: ON (Tab�ŉ���)");
            }
            else
            {
                ShowCursor(TRUE);
                SetWindowTextA(m_window, "�}�E�X�Œ�: OFF (Tab�ŗL��)");
            }
            tabPressed = true;
        }
    }
    else
    {
        static bool tabPressed = false;
        tabPressed = false;
    }



    //  �}�E�X
    POINT mousePos;
    GetCursorPos(&mousePos);                //  ��ʏ�̍��W
    ScreenToClient(m_window, &mousePos);    //  �E�B���h�E�����W�ɕϊ�

    if (m_mouseCaptured)
    {
        if (!m_firstMouse) {
            //  �}�E�X�̈ړ��ʂ��v�Z
            int deltaX = mousePos.x - m_lastMouseX;
            int deltaY = mousePos.y - m_lastMouseY;

            //  ��]���x���X�V
            float mouseSensitivity = 0.002f;

            m_cameraRot.y += deltaX * mouseSensitivity;
            m_cameraRot.x += deltaY * mouseSensitivity;
        }
        else
        {
            m_firstMouse = false;
        }

        RECT rect;
        GetClientRect(m_window, &rect);
        m_lastMouseX = rect.right / 2;
        m_lastMouseY = rect.bottom / 2;

        POINT center = { m_lastMouseX, m_lastMouseY };
        ClientToScreen(m_window, &center);
        SetCursorPos(center.x, center.y);
    }

    else
    {
        m_lastMouseX = mousePos.x;
        m_lastMouseY = mousePos.y;
    }


    // --- 1�񉟂�����������������L�[���͂̊Ǘ� ---
    static std::map<int, bool> keyWasPressed;
    auto IsFirstKeyPress = [&](int vk_code) {
        bool isPressed = GetAsyncKeyState(vk_code) & 0x8000;
        if (isPressed && !keyWasPressed[vk_code]) {
            keyWasPressed[vk_code] = true;
            return true;
        }
        if (!isPressed) {
            keyWasPressed[vk_code] = false;
        }
        return false;
        };

    if (IsFirstKeyPress('1') && !m_isReloading)
    {
        // �v���C���[���s�X�g�����������Ă��邩�m�F���A�����Ă���΂��̃X���b�g�ɐ؂�ւ���
        if (m_primaryWeapon == WeaponType::PISTOL)
        {
            m_currentWeaponSlot = 0;
            SwitchWeapon(WeaponType::PISTOL);
        }
        else if (m_hasSecondaryWeapon && m_secondaryWeapon == WeaponType::PISTOL)
        {
            m_currentWeaponSlot = 1;
            SwitchWeapon(WeaponType::PISTOL);
        }
        // ��: �s�X�g���𗼕��̃X���b�g���甄���Ă��܂����ꍇ�͉����N����Ȃ�
    }
    if (IsFirstKeyPress('2') && !m_isReloading) {
        BuyWeapon(WeaponType::SHOTGUN);
    }
    if (IsFirstKeyPress('3') && !m_isReloading) {
        BuyWeapon(WeaponType::RIFLE);
    }
    if (IsFirstKeyPress('4') && !m_isReloading) {
        BuyWeapon(WeaponType::SNIPER);
    }

    // Q�L�[�ŕ���X���b�v
    if (IsFirstKeyPress('Q') && m_hasSecondaryWeapon && !m_isReloading) {
        // ���݂̃X���b�g�Ƃ͋t�̃X���b�g�ɐ؂�ւ���
        m_currentWeaponSlot = 1 - m_currentWeaponSlot;
        WeaponType newWeapon = (m_currentWeaponSlot == 0) ? m_primaryWeapon : m_secondaryWeapon;
        SwitchWeapon(newWeapon);
    }

    // R�L�[�Ń����[�h�J�n
    if (IsFirstKeyPress('R') && m_currentAmmo < m_maxAmmo && m_reserveAmmo > 0 && !m_isReloading) {
        m_isReloading = true;
        m_reloadTimer = m_weaponStats[m_currentWeapon].reloadTime;
    }



    //  �J������]�̕ω��ʂ��畐��̗h����v�Z
    float rotationDeltaX = m_cameraRot.x - m_lastCameraRotX;
    float rotationDeltaY = m_cameraRot.y - m_lastCameraRotY;

    //  �X�E�F�C���x����
    float swayStrength = 0.5f;
    m_weaponSwayX += rotationDeltaY * swayStrength; //  ���E��]�ŉ��h��
    m_weaponSwayY += rotationDeltaX * swayStrength; //  �㉺��]�ŉ��h��

    //  ��������
    m_weaponSwayX *= 0.9f;
    m_weaponSwayY *= 0.9f;

    //  �O�t���[���̉�]��ێ�
    m_lastCameraRotX = m_cameraRot.x;
    m_lastCameraRotY = m_cameraRot.y;




    if (m_mouseCaptured)
    {
        bool currentMouseState = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

        if (currentMouseState && !m_lastMouseState && !m_isReloading && m_currentAmmo > 0 && m_fireRateTimer <= 0.0f)
        {
            auto& weapon = m_weaponStats[m_currentWeapon];
            m_currentAmmo--;
            m_fireRateTimer = weapon.fireRate;
            CreateMuzzleFlash();
            m_weaponAmmoStatus[m_currentWeapon] = { m_currentAmmo, m_reserveAmmo };

            // �V���b�g�K���͕����̒e�𔭎�
            int pellets = (m_currentWeapon == WeaponType::SHOTGUN) ? 8 : 1;
            for (int p = 0; p < pellets; p++)
            {
                // �ˌ��������v�Z
                DirectX::XMFLOAT3 rayStart = m_cameraPos;
                DirectX::XMFLOAT3 rayDir(
                    sinf(m_cameraRot.y) * cosf(m_cameraRot.x),
                    -sinf(m_cameraRot.x),
                    cosf(m_cameraRot.y) * cosf(m_cameraRot.x)
                );

                // �U�e�̏ꍇ�̓����_���ɍL����
                DirectX::XMFLOAT3 shotDir = rayDir;
                if (m_currentWeapon == WeaponType::SHOTGUN)
                {
                    float spread = 0.1f;
                    shotDir.x += ((float)rand() / RAND_MAX - 0.5f) * spread;
                    shotDir.y += ((float)rand() / RAND_MAX - 0.5f) * spread;
                    shotDir.z += ((float)rand() / RAND_MAX - 0.5f) * spread;
                }

                //  �����蔻��̃`�F�b�N
                bool hit = false;

                if (!hit)
                {
                    for (auto& enemy : m_enemies)
                    {
                        if (!enemy.isAlive)
                            continue;

                        //float distance = // �����v�Z
                        //    if (distance > weapon.range) continue;  // �˒��O

                        if (CheckRayHitsKube(rayStart, rayDir, enemy.position))
                        {
                            hit = true;

                            //  �_���[�W��^����(�w�b�h�V���b�g����)
                            bool isHeadshot = (rayDir.y < -0.3f);
                            int damage = isHeadshot ? 100 : 30;

                            enemy.health -= weapon.damage;

                            if (enemy.health <= 0)
                            {
                                //  �G��|����
                                enemy.isAlive = false;
                                CreateExplosion(enemy.position);
                                m_points += isHeadshot ? 100 : 60;  //  �w�b�h�V���b�g�{�[�i�X
                                m_enemiesKilledThisWave++;

                                m_showDamageDisplay = true;
                                m_damageDisplayTimer = 2.0f;
                                m_damageDisplayPos = enemy.position;
                                m_damageDisplayPos.y += 2.0f;
                                m_damageValue = isHeadshot ? 100 : 60;

                                char debug[256];
                                sprintf_s(debug, "�G���j�I�X�R�A:%d", m_score);
                                SetWindowTextA(m_window, debug);
                            }

                            else
                            {
                                enemy.color.x = 1.0f;
                            }


                            break;
                        }
                    }
                }


                if (!hit)
                {
                    char debug[256];
                    sprintf_s(debug, "�ˌ�...�O�� �X�R�A:%d", m_score);
                    SetWindowTextA(m_window, debug);
                }
            }
        }
        m_lastMouseState = currentMouseState;
    }


    if (GetAsyncKeyState('R') & 0x8000 && m_currentAmmo < m_maxAmmo && m_reserveAmmo > 0 && !m_isReloading)
    {
        if (m_reloadTimer <= 0.0f)
        {
            int needed = m_maxAmmo - m_currentAmmo;
            int reload = min(needed, m_reserveAmmo);
            m_currentAmmo += reload;
            m_reserveAmmo -= reload;
            m_isReloading = false;

            m_weaponAmmoStatus[m_currentWeapon] = { m_currentAmmo, m_reserveAmmo };
        }
    }

    // �����[�h�^�C�}�[�X�V
    if (m_isReloading)
    {
        m_reloadTimer -= 1.0f / 60.0f;
        if (m_reloadTimer <= 0.0f)
        {
            // �����[�h����
            int needed = m_maxAmmo - m_currentAmmo;
            int reload = min(needed, m_reserveAmmo);
            m_currentAmmo += reload;
            m_reserveAmmo -= reload;
            m_isReloading = false;
        }
    }


    if (m_showDamageDisplay)
    {
        m_damageDisplayTimer -= (1.0f / 60.0f); // 60FPS�z��
        if (m_damageDisplayTimer <= 0.0f)
        {
            m_showDamageDisplay = false;
        }
    }


    UpdateParticles();
    UpdateEnemies();


    // �_���[�W�^�C�}�[�X�V
    if (m_damageTimer > 0.0f)
    {
        m_damageTimer -= 1.0f / 60.0f;
        if (m_damageTimer <= 0.0f)
        {
            m_isDamaged = false;
        }
    }

}


void Game::UpdateGameOver()
{
    // �Q�[���I�[�o�[��ʁFR�L�[�Ń��X�^�[�g
    if (GetAsyncKeyState('R') & 0x8000)
    {
        // �Q�[����Ԃ����Z�b�g
        m_score = 0;
        for (int i = 0; i < 3; i++)
        {
            //m_cubesDestroyed[i] = false;
        }
        m_cameraPos = DirectX::XMFLOAT3(0.0f, 2.0f, -5.0f);
        m_gameState = GameState::TITLE;
    }

    char gameOverText[256];
    sprintf_s(gameOverText, "GAME OVER - Final Score: %d - Press R to Restart", m_score);
    SetWindowTextA(m_window, gameOverText);
}

void Game::UpdateFade()
{
    if (!m_fadeActive)
        return;

    float fadeSpeed = 2.0f * (1.0f / 60.0f);

    if (m_fadingIn)
    {
        m_fadeAlpha -= fadeSpeed;
        if (m_fadeAlpha <= 0.0f)
        {
            m_fadeAlpha = 0.0f;
            m_fadeActive = false;
        }
    }
    else
    {
        m_fadeAlpha += fadeSpeed;
        if (m_fadeAlpha >= 1.0f)
        {
            m_fadeAlpha = 1.0f;
            m_fadeActive = false;
        }
    }
}

void Game::UpdateParticles()
{
    float deltaTime = 1.0f / 60.0f; // 60FPS�z��

    // �p�[�e�B�N���̍X�V
    for (auto it = m_particles.begin(); it != m_particles.end();)
    {
        // �ʒu�X�V
        it->position.x += it->velocity.x * deltaTime;
        it->position.y += it->velocity.y * deltaTime;
        it->position.z += it->velocity.z * deltaTime;

        // �d�͓K�p
        it->velocity.y -= 9.8f * deltaTime;

        // ���C�t�^�C���X�V
        it->lifetime -= deltaTime;

        // �t�F�[�h�A�E�g
        float alpha = it->lifetime / it->maxLifetime;
        it->color.w = alpha;

        // �����؂�̃p�[�e�B�N�����폜
        if (it->lifetime <= 0.0f)
        {
            it = m_particles.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // �e���t���b�V���^�C�}�[�X�V
    if (m_showMuzzleFlash)
    {
        m_muzzleFlashTimer -= deltaTime;
        if (m_muzzleFlashTimer <= 0.0f)
        {
            m_showMuzzleFlash = false;
        }
    }
}


void Game::RenderDamageFlash()
{
    auto context = m_d3dContext.Get();

    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(
        (float)m_outputWidth, (float)m_outputHeight, 0.1f, 10.0f);

    m_effect->SetView(view);
    m_effect->SetProjection(projection);
    m_effect->SetWorld(DirectX::XMMatrixIdentity());
    m_effect->SetVertexColorEnabled(true);

    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());

    auto primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
    primitiveBatch->Begin();

    float alpha = m_damageTimer;  // �^�C�}�[�ɉ����ăt�F�[�h�A�E�g
    DirectX::XMFLOAT4 bloodColor(0.8f, 0.0f, 0.0f, alpha * 0.3f);
    float halfWidth = m_outputWidth * 0.5f;
    float halfHeight = m_outputHeight * 0.5f;

    // ��ʒ[��Ԃ�
    float borderSize = 100.0f;

    // ��
    DirectX::VertexPositionColor v1(DirectX::XMFLOAT3(-halfWidth, halfHeight, 1.0f), bloodColor);
    DirectX::VertexPositionColor v2(DirectX::XMFLOAT3(halfWidth, halfHeight, 1.0f), bloodColor);
    DirectX::VertexPositionColor v3(DirectX::XMFLOAT3(halfWidth, halfHeight - borderSize, 1.0f), bloodColor);
    DirectX::VertexPositionColor v4(DirectX::XMFLOAT3(-halfWidth, halfHeight - borderSize, 1.0f), bloodColor);

    primitiveBatch->DrawQuad(v1, v2, v3, v4);

    // ����3�ӂ����l��...�i�ȗ��j

    primitiveBatch->End();
}


void Game::RenderTitle()
{

}

void Game::RenderGameOver()
{

}

void Game::RenderFade()
{
    if (m_fadeAlpha <= 0.0f)
        return;

    // �t�F�[�h�p��2D�`��ݒ�
    auto context = m_d3dContext.Get();

    // 2D�p�̒P�ʍs��
    DirectX::XMMATRIX view = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX projection = DirectX::XMMatrixOrthographicLH(
        (float)m_outputWidth, (float)m_outputHeight, 0.1f, 10.0f);

    m_effect->SetView(view);
    m_effect->SetProjection(projection);
    m_effect->SetWorld(DirectX::XMMatrixIdentity());

    // ���_�J���[�𖳌������ĒP�F�`�惂�[�h��
    m_effect->SetVertexColorEnabled(false);
    DirectX::XMVECTOR diffuseColor = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, m_fadeAlpha);
    m_effect->SetDiffuseColor(diffuseColor);

    m_effect->Apply(context);
    context->IASetInputLayout(m_inputLayout.Get());

    // �t���X�N���[���̎l�p�`��`��
    auto primitiveBatch = std::make_unique<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>(context);
    primitiveBatch->Begin();

    DirectX::XMFLOAT4 fadeColor(0.0f, 0.0f, 0.0f, m_fadeAlpha);
    float halfWidth = m_outputWidth * 0.5f;
    float halfHeight = m_outputHeight * 0.5f;

    // ��ʑS�̂𕢂��l�p�`
    DirectX::VertexPositionColor v1(DirectX::XMFLOAT3(-halfWidth, -halfHeight, 1.0f), fadeColor);
    DirectX::VertexPositionColor v2(DirectX::XMFLOAT3(-halfWidth, halfHeight, 1.0f), fadeColor);
    DirectX::VertexPositionColor v3(DirectX::XMFLOAT3(halfWidth, halfHeight, 1.0f), fadeColor);
    DirectX::VertexPositionColor v4(DirectX::XMFLOAT3(halfWidth, -halfHeight, 1.0f), fadeColor);

    // �O�p�`2�Ŏl�p�`���\��
    primitiveBatch->DrawTriangle(v1, v2, v3);
    primitiveBatch->DrawTriangle(v1, v3, v4);

    primitiveBatch->End();

    // ���_�J���[���ēx�L����
    m_effect->SetVertexColorEnabled(true);
}




void Game::SpawnEnemy()
{
    OutputDebugStringA("SpawnEnemy called\n");

    if (m_enemies.size() >= m_maxEnemies)
        return;
    OutputDebugStringA("Enemy count check passed\n");

    Enemy enemy;

    // �����_���Ȉʒu�ɐ����i�v���C���[���痣�ꂽ�ꏊ�j
    float angle = (float)rand() / RAND_MAX * 2.0f * 3.14159f;
    float distance = 10.0f + (float)rand() / RAND_MAX * 10.0f;

    enemy.position.x = m_cameraPos.x + cosf(angle) * distance;
    enemy.position.y = 1.0f;
    enemy.position.z = m_cameraPos.z + sinf(angle) * distance;

    // �������x
    enemy.velocity.x = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;
    enemy.velocity.y = 0.0f;
    enemy.velocity.z = ((float)rand() / RAND_MAX - 0.5f) * 4.0f;

    enemy.color = DirectX::XMFLOAT4(
        0.3f + (float)rand() / RAND_MAX * 0.2f,  // �Â���
        0.4f + (float)rand() / RAND_MAX * 0.2f,  // �Â���  
        0.2f,  // �Ⴂ��
        1.0f
    );

    // HP���E�F�[�u�ɉ����Đݒ�
    enemy.maxHealth = 100 + (m_currentWave - 1) * 50;  // Wave1: 100HP, Wave2: 150HP...
    enemy.health = enemy.maxHealth;

    enemy.isAlive = true;
    enemy.moveTimer = 0.0f;
    enemy.nextDirectionChange = 2.0f + (float)rand() / RAND_MAX * 3.0f;

    m_enemies.push_back(enemy);
}