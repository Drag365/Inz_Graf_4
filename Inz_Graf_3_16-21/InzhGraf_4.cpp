#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "engine_common.h"
#include "util.h"
#include "pipeline.h"
#include "camera.h"
#include "texture.h"
#include "lighting_technique.h"
#include "glut_backend.h"
#include "mesh.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1200


class Tutorial26 : public ICallbacks
{
public:

    Tutorial26()
    {
        m_pLightingTechnique = NULL; // Указатель на технику освещения
        m_pGameCamera = NULL; // Указатель на камеру
        m_pSphereMesh = NULL; // Указатель на меш
        m_scale = 0.0f; // Значение масштаба
        m_pTexture = NULL; // Указатель на текстуру
        m_pNormalMap = NULL; // Указатель на текстуру нормалей
        m_pTrivialNormalMap = NULL; // Указатель на тривиальную текстуру нормалей

        // Свойства направленного источника света
        m_dirLight.AmbientIntensity = 0.2f;    // Интенсивность фонового света
        m_dirLight.DiffuseIntensity = 0.8f;    // Интенсивность диффузного света
        m_dirLight.Color = Vector3f(1.0f, 1.0f, 1.0f);    // Цвет света
        m_dirLight.Direction = Vector3f(1.0f, 0.0f, 0.0f); // Направление света

        // Свойства перспективной проекции
        m_persProjInfo.FOV = 60.0f;            // Угол обзора проекции
        m_persProjInfo.Height = WINDOW_HEIGHT; // Высота проекции
        m_persProjInfo.Width = WINDOW_WIDTH;   // Ширина проекции
        m_persProjInfo.zNear = 1.0f;           // Ближняя граница отсечения проекции
        m_persProjInfo.zFar = 100.0f;          // Дальняя граница отсечения проекции

        m_bumpMapEnabled = true;               // Флаг использования бамп-мэппинга, по умолчанию установлен на true
    }


    ~Tutorial26()
    {
        SAFE_DELETE(m_pLightingTechnique);
        SAFE_DELETE(m_pGameCamera);
        SAFE_DELETE(m_pSphereMesh);
        SAFE_DELETE(m_pTexture);
        SAFE_DELETE(m_pNormalMap);
        SAFE_DELETE(m_pTrivialNormalMap);
    }


    bool Init()
    {
        // Создание камеры с заданным положением, целевой точкой и направлением вверх
        Vector3f Pos(0.5f, 1.025f, 0.25f); // Положение камеры
        Vector3f Target(0.0f, -0.5f, 1.0f); // Целевая точка камеры
        Vector3f Up(0.0, 1.0f, 0.0f); // Направление "вверх" камеры
        m_pGameCamera = new Camera(WINDOW_WIDTH, WINDOW_HEIGHT, Pos, Target, Up);

        // Создание объекта техники освещения
        m_pLightingTechnique = new LightingTechnique();

        // Инициализация техники освещения
        if (!m_pLightingTechnique->Init()) {
            printf("Error initializing the lighting technique\n");
            return false;
        }

        // Включение техники освещения
        m_pLightingTechnique->Enable();

        // Установка параметров направленного источника света
        m_pLightingTechnique->SetDirectionalLight(m_dirLight);

        // Установка текстурных слотов для цветовой и нормальной текстур
        m_pLightingTechnique->SetColorTextureUnit(0);
        m_pLightingTechnique->SetNormalMapTextureUnit(2);

        // Создание объекта меша
        m_pSphereMesh = new Mesh();

        // Загрузка меша из файла
        if (!m_pSphereMesh->LoadMesh("C:/Content/box.obj")) {
            return false;
        }

        // Создание объекта текстуры и загрузка цветовой текстуры
        m_pTexture = new Texture(GL_TEXTURE_2D, "C:/Content/bricks.jpg");

        if (!m_pTexture->Load()) {
            return false;
        }

        // Привязка цветовой текстуры к текстурному слоту 0
        m_pTexture->Bind(COLOR_TEXTURE_UNIT);

        // Создание объекта текстуры и загрузка текстуры нормалей
        m_pNormalMap = new Texture(GL_TEXTURE_2D, "C:/Content/normal_map.jpg");

        if (!m_pNormalMap->Load()) {
            return false;
        }

        // Создание объекта текстуры и загрузка тривиальной текстуры нормалей
        m_pTrivialNormalMap = new Texture(GL_TEXTURE_2D, "C:/Content/normal_up.jpg");

        if (!m_pTrivialNormalMap->Load()) {
            return false;
        }

        return true;
    }


    void Run()
    {
        GLUTBackendRun(this);
    }


    // Виртуальная функция отрисовки сцены
    virtual void RenderSceneCB()
    {
        // Обновление положения камеры
        m_pGameCamera->OnRender();

        // Увеличение масштаба объекта
        m_scale += 0.01f;

        // Очистка буфера цвета и буфера глубины
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Включение техники освещения
        m_pLightingTechnique->Enable();

        // Создание матрицы трансформации и установка параметров камеры и перспективной проекции
        Pipeline p;
        p.Rotate(0.0f, m_scale, 0.0f);                  // Поворот объекта
        p.WorldPos(0.0f, 0.0f, 3.0f);                   // Положение объекта
        p.SetCamera(m_pGameCamera->GetPos(), m_pGameCamera->GetTarget(), m_pGameCamera->GetUp());  // Параметры камеры
        p.SetPerspectiveProj(m_persProjInfo);           // Параметры перспективной проекции

        // Привязка цветовой текстуры к текстурному слоту 0
        m_pTexture->Bind(COLOR_TEXTURE_UNIT);

        // Если включено использование карты нормалей, привязываем её к текстурному слоту 1, иначе - привязываем тривиальную карту нормалей
        if (m_bumpMapEnabled)
        {
            m_pNormalMap->Bind(NORMAL_TEXTURE_UNIT);
        }
        else
        {
            m_pTrivialNormalMap->Bind(NORMAL_TEXTURE_UNIT);
        }

        // Установка матриц трансформации и отрисовка меша
        m_pLightingTechnique->SetWVP(p.GetWVPTrans());
        m_pLightingTechnique->SetWorldMatrix(p.GetWorldTrans());
        m_pSphereMesh->Render();

        // Отображение отрисованного содержимого на экране
        glutSwapBuffers();
    }


    virtual void IdleCB()
    {
        RenderSceneCB();
    }


    virtual void SpecialKeyboardCB(int Key, int x, int y)
    {
        m_pGameCamera->OnKeyboard(Key);
    }


    virtual void KeyboardCB(unsigned char Key, int x, int y)
    {
        switch (Key) {
        case 'q':
            glutLeaveMainLoop();
            break;

        case 'b':
            m_bumpMapEnabled = !m_bumpMapEnabled;
            break;
        }
    }


    virtual void PassiveMouseCB(int x, int y)
    {
        m_pGameCamera->OnMouse(x, y);
    }

private:

    LightingTechnique* m_pLightingTechnique;
    Camera* m_pGameCamera;
    float m_scale;
    DirectionalLight m_dirLight;
    Mesh* m_pSphereMesh;
    Texture* m_pTexture;
    Texture* m_pNormalMap;
    Texture* m_pTrivialNormalMap;
    PersProjInfo m_persProjInfo;
    bool m_bumpMapEnabled;
};


// Главная функция программы
int main(int argc, char** argv)
{
    // Инициализация бэкенда GLUT
    GLUTBackendInit(argc, argv);

    // Создание окна с заданными параметрами
    if (!GLUTBackendCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 32, false, "Tutorial 26")) {
        return 1;
    }

    // Создание экземпляра приложения Tutorial26
    Tutorial26* pApp = new Tutorial26();

    // Инициализация приложения
    if (!pApp->Init()) {
        return 1;
    }

    // Запуск цикла обработки сообщений и отрисовки кадров
    pApp->Run();

    // Освобождение памяти, занятой экземпляром приложения
    delete pApp;

    // Возвращение кода успешного завершения программы
    return 0;
}