// Add these lines to .pro:
// INCLUDEPATH += "E:\Libs\assimp-5.0.1-mingw-32bit\include"
// LIBS += -L"E:\Libs\assimp-5.0.1-mingw-32bit\lib"
// LIBS += -lopengl32 -lassimp -lIrrXML -lzlibstatic

#ifdef _WIN32
#include <windows.h>
extern "C" __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
#endif

#include <QtWidgets/QApplication>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QMessageBox>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtCore/QDebug>
#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class OpenGLWidget : public QOpenGLWidget {
public:
    OpenGLWidget(QWidget *parent = nullptr) : QOpenGLWidget (parent) {
        setWindowTitle("Qt C++, OpenGL");
        resize(300, 300);
    }
private:
    QOpenGLBuffer m_vertPosBuffer;
    QOpenGLShaderProgram m_program;
    int m_numVertices;
    void initializeGL() override {
        glClearColor(0.1f, 0.1f, 0.1f, 1.f);
        glEnable(GL_DEPTH_TEST);
        Assimp::Importer importer;
        const char *path = "Models/PlaneBlender8.dae";
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            qDebug() << "Assimp Error:" << importer.GetErrorString();
            QMessageBox::critical(this, "Assimp Error:", importer.GetErrorString());
            return;
        }
        m_numVertices = scene->mMeshes[0]->mNumVertices;
        float vertPositions[m_numVertices * 3];
        int vertPosIndex = 0;
        for (int i = 0; i < m_numVertices; i++) {
            vertPositions[vertPosIndex++] = scene->mMeshes[0]->mVertices[i].x;
            vertPositions[vertPosIndex++] = scene->mMeshes[0]->mVertices[i].y;
            vertPositions[vertPosIndex++] = scene->mMeshes[0]->mVertices[i].z;
//            qDebug() << scene->mMeshes[0]->mVertices[i].x << ", "
//                     << scene->mMeshes[0]->mVertices[i].y << ", "
//                     << scene->mMeshes[0]->mVertices[i].z;
        }
        m_vertPosBuffer.create();
        m_vertPosBuffer.bind();
        m_vertPosBuffer.allocate(vertPositions, sizeof(vertPositions));
        const char *vertShaderSrc =
                "attribute vec3 aPosition;"
                "uniform mat4 uModelMatrix;"
                "void main()"
                "{"
                "    gl_Position = uModelMatrix * vec4(aPosition, 1.0);"
                "}";
        const char *fragShaderSrc =
                "void main()"
                "{"
                "    gl_FragColor = vec4(0.5, 0.2, 0.7, 1.0);"
                "}";
        m_program.create();
        m_program.addShaderFromSourceCode(QOpenGLShader::Vertex, vertShaderSrc);
        m_program.addShaderFromSourceCode(QOpenGLShader::Fragment, fragShaderSrc);
        m_program.link();
        QMatrix4x4 modelMatrix;
        modelMatrix.scale(0.5);
        m_program.bind();
        m_program.setUniformValue("uModelMatrix", modelMatrix);
        m_program.setAttributeBuffer("aPosition", GL_FLOAT, 0, 3);
        m_program.enableAttributeArray("aPosition");
    }
    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
    }
    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    OpenGLWidget w;
    w.show();
    return a.exec();
}
