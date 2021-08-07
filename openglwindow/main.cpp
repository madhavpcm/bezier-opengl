/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "openglwindow.h"

#include <QGuiApplication>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QScreen>
#include <QtMath>
#include <QFile>
#include <iostream>
//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    using OpenGLWindow::OpenGLWindow;

    void initialize() override;
    void render() override;

private:
    GLint m_posAttr = 0;
    GLint m_colAttr = 0;
    GLint m_matrixUniform = 0;
    GLint m_coord2d = 0;
    QOpenGLShaderProgram *m_program = nullptr;
    int m_frame = 0;
};
//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    QSurfaceFormat format;
    format.setSamples(16);

    TriangleWindow window;
    window.setFormat(format);
    window.resize(640, 480);
    window.show();

    window.setAnimating(false);

    return app.exec();
}
//! [2]




//! [4]
void TriangleWindow::initialize()
{
    m_program = new QOpenGLShaderProgram(this);
    QFile vertexShaderSource(":/shaders/shaders/vertex.vert"),
          fragmentShaderSource(":/shaders/shaders/fragment.frag");
    if(!vertexShaderSource.open(QFile::ReadOnly |
                     QFile::Text))
       {
           qDebug() << " Could not open Vertex shader for reading";
           return;
       }
    if(!fragmentShaderSource.open(QFile::ReadOnly |
                     QFile::Text))
       {
           qDebug() << " Could not open Fragment shader for reading";
           return;
       }
    QByteArray vs = vertexShaderSource.readAll(),
               fs = fragmentShaderSource.readAll();
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vs);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fs);
    m_program->link();
    //m_posAttr = m_program->attributeLocation("posAttr");
    //Q_ASSERT(m_posAttr != -1);
    //m_colAttr = m_program->attributeLocation("colAttr");
    //Q_ASSERT(m_colAttr != -1);
    m_matrixUniform = m_program->uniformLocation("matrix");
    Q_ASSERT(m_matrixUniform != -1);
    m_coord2d = m_program->attributeLocation("coord2d");
    Q_ASSERT(m_coord2d != -1);

}
//! [4]

//! [5]
void TriangleWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();

    QMatrix4x4 matrix;
    matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -2);
   // matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);

    m_program->setUniformValue(m_matrixUniform, matrix);

    static const GLfloat vertices[] = {
         0.0f,  0.707f,
        -0.5f, -0.5f,
         0.5f, -0.5f
    };

    static const GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    static point graph[2000];

    for(int i = 0; i < 2000; i++) {
      float x = (i - 1000.0) / 100.0;
      graph[i].x = x;
      graph[i].y = sin(x * 10.0) / (1.0 + x * x);
    }

    GLuint vbo;
    glGenBuffers(1,&vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof graph, graph, GL_DYNAMIC_DRAW);

    //glVertexAttribPointer(m_posAttr, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    //glVertexAttribPointer(m_colAttr, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glVertexAttribPointer(m_coord2d, 2, GL_FLOAT,GL_FALSE,0,0);
    glEnableVertexAttribArray(m_coord2d);
    glEnableVertexAttribArray(m_colAttr);

    glDrawArrays(GL_LINE_STRIP, 0, 2000);
    //glDisableVertexAttribArray(m_colAttr);
    glDisableVertexAttribArray(m_coord2d);


    m_program->release();

    ++m_frame;
}
//! [5]
