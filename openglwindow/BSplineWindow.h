#ifndef BSPLINEWINDOW_H
#define BSPLINEWINDOW_H

#include "openglwindow.h"
#include <qopenglfunctions_4_5_core.h>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QScreen>
#include <QtMath>
#include <QFile>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <Qt>
#include <QDebug>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <atomic>
//! [1]
class BSplineWindow : public OpenGLWindow
{
public:
    using OpenGLWindow::OpenGLWindow;
    //explicit BSplineWindow(QWindow *parent = nullptr);

    void initialize() override;
    void render() override;
    void getFirstControlPoints();
    void getCurveControlPoints();
    std::pair<int,int> closestKnot(glm::vec2 &v);
    std::vector<glm::vec3> updateControlPoints(std::vector<glm::vec3> & rhs);
    GLfloat getBezier(GLfloat x, GLfloat k1, GLfloat c1, GLfloat c2, GLfloat k2);
    void win2glcoord(glm::vec2 & v);
    void dragMouse(int indx,glm::vec2 &nmc);
    void nearestPoint(glm::vec3 point, std::vector<glm::vec3>& list);
private:
    GLint m_posAttr = 0;
    GLint m_colAttr = 0;
    GLint m_matrixUniform = 0;
    GLint m_coord2d = 0;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e)	override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *e) override;

    std::atomic_bool m_isknotselected;
    //void mouseDoubleClickEvent(QMouseEvent *e) override;

    std::vector<glm::vec3> m_knots;
    std::vector<glm::vec3> m_firstControlPoints;
    std::vector<glm::vec3> m_secondControlPoints;
    std::vector<std::vector<glm::vec3>> m_feedback;//all coords after rendering

    QOpenGLShaderProgram *m_program = nullptr;
    int m_frame = 0;
};
//! [1]
#endif // BSPLINEWINDOW_H
