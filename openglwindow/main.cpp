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
#include <qopenglfunctions_4_5_core.h>
#include <QGuiApplication>
#include <QMatrix4x4>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QScreen>
#include <QtMath>
#include <QFile>
#include <iostream>
#include <QMouseEvent>
#include <Qt>
#include <QDebug>
#include <glm/vec2.hpp>
#include <atomic>
#include <glm/glm.hpp>
//! [1]
class TriangleWindow : public OpenGLWindow
{
public:
    using OpenGLWindow::OpenGLWindow;
    //explicit TriangleWindow(QWindow *parent = nullptr);

    void initialize() override;
    void render() override;
    void getFirstControlPoints();
    void getCurveControlPoints();
    int closestKnot(glm::vec2 &v);
    std::vector<glm::vec3> updateControlPoints(std::vector<glm::vec3> & rhs);
    void win2glcoord(glm::vec2 & v);
    void dragMouse(int indx,glm::vec2 &nmc);
private:
    GLint m_posAttr = 0;
    GLint m_colAttr = 0;
    GLint m_matrixUniform = 0;
    GLint m_coord2d = 0;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e)	override;
    void mouseMoveEvent(QMouseEvent *e) override;

    std::atomic_bool m_isknotselected;
    //void mouseDoubleClickEvent(QMouseEvent *e) override;

    std::vector<glm::vec3> m_knots      ;
    std::vector<glm::vec3> m_firstControlPoints;
    std::vector<glm::vec3> m_secondControlPoints;
    QOpenGLShaderProgram *m_program = nullptr;
    int m_frame = 0;
};
//! [1]

//! [2]
int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    //std::cout<<"sd;lfkajsf";
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

void TriangleWindow::dragMouse(int indx, glm::vec2 &nmc){
        m_knots[indx] = glm::vec3(nmc, 0);
        std::sort(m_knots.begin(), m_knots.end(), [] (const glm::vec3& a,const glm::vec3& b)
        { return a.x < b.x; });

}

int TriangleWindow::closestKnot(glm::vec2 &v){
    GLfloat min= 3.402823466E38;
    uint32_t indx=-1;
   for(size_t i =0 ; i< m_knots.size(); i++){
        GLfloat x=glm::distance(m_knots[i],glm::vec3(v,0));
        if(x < min){
            min=x;
            indx=i;
        }
   }
   if(min <= 0.6f)
        return indx;
   else
        return -1;
}

void TriangleWindow::win2glcoord(glm::vec2 & v){

        v.x =( v.x / width()) * 20;
        v.y =( v.y / height())* 20;
        v.x -= 10;
        v.y -= 10;
        v.y *= -1;
}
void TriangleWindow::mousePressEvent(QMouseEvent *e){
   if(e->button() == Qt::LeftButton && !m_isknotselected){
        m_isknotselected = true;

    }
   else{
       m_isknotselected = false;
   }

}

void TriangleWindow::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
       m_isknotselected = false;
    }
}
void TriangleWindow::mouseMoveEvent(QMouseEvent *e){

   if( m_isknotselected  ){
        QPoint mousecoords=mapFromGlobal(QCursor::pos());
        glm::vec2 nmc={mousecoords.x() ,mousecoords.y()};
        win2glcoord(nmc);
        int cindex = closestKnot(nmc);

        if(cindex < 0 ){
            m_knots.push_back(glm::vec3(nmc.x,nmc.y,0));//add new control point if no close one exists
            getCurveControlPoints();
            renderNow();

        }else{
            dragMouse(cindex,nmc);
            getCurveControlPoints();
            renderNow();
        }
        //std::cout << nmc.x << " :: " << nmc.y << " \n";
    }
}
//! [4]
void TriangleWindow::initialize()
{
    m_isknotselected = false;
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
    m_colAttr = m_program->attributeLocation("colAttr");
    Q_ASSERT(m_colAttr != -1);
    m_matrixUniform = m_program->uniformLocation("matrix");
    Q_ASSERT(m_matrixUniform != -1);
    m_coord2d = m_program->attributeLocation("coord2d");
    Q_ASSERT(m_coord2d != -1);

    static std::vector<glm::vec3> ctrlpoints = {
            { -4.0, -4.0,0.0}, { -2.0, 4.0,0.0},
            {2.0, -4.0,0.0}, {4.0, 4.0,0.0}
    };
    for(glm::vec3 v : ctrlpoints)
        m_knots.push_back(v);

    getCurveControlPoints();
}
//! [4]

//! [5]
void TriangleWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    glClear(GL_COLOR_BUFFER_BIT);

    //std::cout << "rendering\n";
    m_program->bind();

    QMatrix4x4 matrix;
    matrix.ortho(-10.0,10,-10,10,0.1f,100.0f);
    //matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    matrix.translate(0, 0, -100);
   // matrix.rotate(100.0f * m_frame / screen()->refreshRate(), 0, 1, 0);
    m_program->setUniformValue(m_matrixUniform, matrix);

    glm::vec3 color = {1,0,0};

    static point graph[2000];
    QOpenGLBuffer bezBuffer((QOpenGLBuffer(QOpenGLBuffer::VertexBuffer)));
    //bool x=bezBuffer.create();
    //x=bezBuffer.bind();


    for(int i = 0; i < 2000; i++) {
      float x = (i - 1000.0) / 100.0;
      graph[i].x = x;
      graph[i].y = sin(x * 10.0) / (1.0 + x * x);
    }
    bezBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    bezBuffer.allocate(graph,2000 *sizeof (point));
    m_program->setAttributeBuffer(m_coord2d,GL_FLOAT,0,2,0);
    m_program->enableAttributeArray(m_coord2d);
    m_program->setAttributeBuffer(m_colAttr, GL_FLOAT, 0,3,0);
    m_program->enableAttributeArray(m_colAttr);
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glDrawArrays(GL_LINE_STRIP, 0, 2000);
    m_program->disableAttributeArray(m_colAttr);
    m_program->disableAttributeArray(m_coord2d);
    std::vector<glm::vec3> curve(4);

    m_program->setAttributeBuffer(m_colAttr, GL_FLOAT, 0,3,0);
    m_program->enableAttributeArray(m_colAttr);
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);

    for(size_t i=0; i< m_firstControlPoints.size() ; i++){
        curve[0]=m_knots[i];
        curve[1]=(m_firstControlPoints[i]);
        curve[2]=(m_secondControlPoints[i]);
        curve[3]=(m_knots[i+1]);
        glEnable(GL_MAP1_VERTEX_3);
        glMap1f(GL_MAP1_VERTEX_3,0.0,1.0, 3,4,&curve[0][0]);
      //glColor3f( 1.0, 1.0, 0.0 );
        glBegin(GL_LINE_STRIP);
            for(int i=0; i < 10000; i++)
                glEvalCoord1f((GLfloat) i/ 10000.0);
        glEnd();
        glDisable(GL_MAP1_VERTEX_3);
    }
    color = {0,1,0};
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glPointSize(5.0);
    glBegin(GL_POINTS);
         for(glm::vec3 v : m_firstControlPoints)
            glVertex3f(v.x,v.y,v.z);
    glEnd();

    color = {0,0,1};
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glBegin(GL_POINTS);
    for(glm::vec3 v : m_secondControlPoints)
       glVertex3f(v.x,v.y,v.z);
    glEnd();

    color = {1,1,1};
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glBegin(GL_POINTS);
        for(glm::vec3 v : m_knots)
            glVertex3f(v.x,v.y,v.z);
    glEnd();
    m_program->disableAttributeArray(m_colAttr);
    bezBuffer.release();
    m_program->release();

    ++m_frame;
}

void TriangleWindow::getCurveControlPoints(){
    if(!m_knots.size()){
        qDebug()<<"knots is empty"; return;
    }
    if(m_knots.size() < 2){
        qDebug() << "knots needs at least 2 points"; return;
    }
    if(m_knots.size() == 2){
        m_firstControlPoints[0]=(glm::vec3{
                                              (2 *m_knots[0].x + m_knots[1].x )/3,
                                              (2 *m_knots[0].y + m_knots[1].y )/3,
                                               0.0
                                          });
        m_secondControlPoints[1]=(glm::vec3{
                                              (2*m_firstControlPoints[0].x - m_knots[0].x),
                                               (2*m_firstControlPoints[0].y - m_knots[0].y),
                                               0.0
                                           });
        return;
    }
    size_t n=m_knots.size()-1;
    std::vector<glm::vec3> rhs(m_knots.size()-1);
    if(m_firstControlPoints.size() != n)
           m_firstControlPoints = std::vector<glm::vec3> (n);
    if(m_secondControlPoints.size() != n)
            m_secondControlPoints = std::vector<glm::vec3> (n);// Set right hand side X values
    for (size_t i = 1; i <n-1; ++i){
        rhs[i].x = 4 * m_knots[i].x + 2 * m_knots[i + 1].x;
        rhs[i].y = 4 * m_knots[i].y + 2 * m_knots[i + 1].y;
        rhs[i].z=0;

    }
    rhs[0].y = m_knots[0].y + 2 * m_knots[1].y;
    rhs[n - 1].y = (8 * m_knots[n - 1].y + m_knots[n].y) / 2.0;
    rhs[0].x = m_knots[0].x + 2 * m_knots[1].x;
    rhs[n - 1].x = (8 * m_knots[n - 1].x + m_knots[n].x) / 2.0;
    // Get first control points X-values
    std::vector<glm::vec3> ctrl = updateControlPoints(rhs);

    // Set right hand side Y values

    // Get first control points Y-values


    // Fill output arrays.
    for (size_t i = 0; i < n; ++i)
    {
        // First control point
        m_firstControlPoints[i]=(glm::vec3(ctrl[i].x, ctrl[i].y,0));
        // Second control point
        if (i < n - 1)
            m_secondControlPoints[i]=(glm::vec3(2 * m_knots[i + 1].x - ctrl[i + 1].x,
                                                         2 *m_knots[i + 1].y - ctrl[i + 1].y,
                                                         0));
        else
            m_secondControlPoints[i]=(glm::vec3((m_knots[n].x + ctrl[n - 1].x) / 2,
                                                         (m_knots[n].y + ctrl[n - 1].y) / 2,
                                                          0));
    }

}

//! [5]
std::vector<glm::vec3> TriangleWindow::updateControlPoints(std::vector<glm::vec3> & rhs){
    size_t n = rhs.size();
    std::vector<glm::vec3> solution(n); // Solution vector.
    std::vector<glm::vec3> tmp(n); // Solution vector.

    glm::vec3 b={2.0,2.0,0};
    solution[0] = rhs[0] / b;
    for (size_t i = 1; i < n; i++) // Decomposition and forward substitution.
    {
        tmp[i].x = 1 / b.x;
        tmp[i].y = 1/ b.y;
        b.x = (i < n - 1 ? 4.0 : 3.5) - tmp[i].x;
        solution[i].x = (rhs[i].x - solution[i - 1].x) / b.x;
        solution[i].y = (rhs[i].y - solution[i - 1].y) / b.y;

    }
    for (size_t i = 1; i < n; i++)
        solution[n - i - 1] -= tmp[n - i] * solution[n - i]; // Backsubstitution.

    return solution;
}
