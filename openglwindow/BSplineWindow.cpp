
#include "BSplineWindow.h"

void BSplineWindow::mousePressEvent(QMouseEvent *e){
    if(e->button() == Qt::RightButton){
        glm::vec2 nmc(e->pos().x(),e->pos().y());
        win2glcoord(nmc);
        std::pair<int,int> cindex = closestKnot(nmc);

        if(cindex.first !=0){
            return;
        }
        else{
            if(m_knots.size() >2){
                m_knots.erase(m_knots.begin() + cindex.second);
                getCurveControlPoints();
                renderNow();

            }
        }
    }
   if(e->button() == Qt::LeftButton && !m_isknotselected){
        m_isknotselected = true;

    }
   else{
       m_isknotselected = false;
   }

}

void BSplineWindow::mouseReleaseEvent(QMouseEvent *e){
    if(e->button() == Qt::LeftButton){
       m_isknotselected = false;
    }
}

void BSplineWindow::mouseMoveEvent(QMouseEvent *e){

   if( m_isknotselected  ){
        glm::vec2 nmc(e->pos().x(),e->pos().y());
        win2glcoord(nmc);
        std::pair<int,int> cindex  = closestKnot(nmc);

        if(cindex.second == 0 ){
            dragMouse(cindex.first, nmc);
            getCurveControlPoints();
            renderNow();
        }else if (cindex.second== 1){
            m_firstControlPoints[cindex.first] = glm::vec3(nmc,0);
            renderNow();
        }else if (cindex.second== 2) {
            m_secondControlPoints[cindex.first] = glm::vec3(nmc,0);
            renderNow();
        }
        else if (cindex.first < 0){
            m_knots.push_back(glm::vec3(nmc.x,nmc.y,0));//add new control point if no close one exists
            getCurveControlPoints();
            renderNow();
        }

        //std::cout << nmc.x << " :: " << nmc.y << " \n";
    }
}

void BSplineWindow::mouseDoubleClickEvent(QMouseEvent *e){
    if( e->button() == Qt::LeftButton)    {
        getCurveControlPoints();
        renderNow();
    }
}

void BSplineWindow::initialize()
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

    //checking uniforms, primitive check as things are fairly simple

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

void BSplineWindow::render()
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

    std::vector<glm::vec3> curve(4);

    m_program->setAttributeBuffer(m_colAttr, GL_FLOAT, 0,3,0);
    m_program->enableAttributeArray(m_colAttr);
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);

    for(size_t i=0; i< m_firstControlPoints.size() ; i++){
        std::vector<glm::vec3> feedback(1000);
        curve[0]=m_knots[i];
        curve[1]=(m_firstControlPoints[i]);
        curve[2]=(m_secondControlPoints[i]);
        curve[3]=(m_knots[i+1]);
        glColor3f( 1.0, 1.0, 0.0 );
        glBegin(GL_LINE_STRIP);
            for(int i=0; i < 1000; i++){
                feedback[i] = glm::vec3(getBezier(i/1000.0,curve[0].x,curve[1].x,curve[2].x,curve[3].x),
                                        getBezier(i/1000.0,curve[0].y,curve[1].y,curve[2].y,curve[3].y),
                                        0);
                glVertex3fv(&feedback[i][0]);
            }
        glEnd();
        glDisable(GL_MAP1_VERTEX_3);


    }//Control Points 1 in green
    color = {0,1,0};
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glPointSize(5.0);
    glBegin(GL_POINTS);
         for(glm::vec3 v : m_firstControlPoints)
            glVertex3f(v.x,v.y,v.z);
    glEnd();
    //Control Points 2 in blue
    color = {0,0,1};
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glBegin(GL_POINTS);
    for(glm::vec3 v : m_secondControlPoints)
       glVertex3f(v.x,v.y,v.z);
    glEnd();
    //Knots or the joining points in white
    color = {1,1,1};
    m_program->setAttributeValue(m_colAttr,color.x,color.y,color.z);
    glBegin(GL_POINTS);
        for(glm::vec3 v : m_knots)
            glVertex3f(v.x,v.y,v.z);
    glEnd();
    m_program->disableAttributeArray(m_colAttr);
    //bezBuffer.release();
    m_program->release();

    ++m_frame;
}

void BSplineWindow::dragMouse(int indx, glm::vec2 &nmc){
        m_knots[indx] = glm::vec3(nmc, 0);
        std::sort(m_knots.begin(), m_knots.end(), [] (const glm::vec3& a,const glm::vec3& b)
        { return a.x < b.x; });

}

std::pair<int,int> BSplineWindow::closestKnot(glm::vec2 &v){
    GLfloat min= 3.402823466E38;
    uint32_t indx=-1;
    int id=-1;
   for(size_t i =0 ; i< m_knots.size(); i++){
        GLfloat x=glm::distance(m_knots[i],glm::vec3(v,0));
        if(x < min){
            min=x;
            indx=i;
            id=0;
        }
   }
   for(size_t i=0; i < m_firstControlPoints.size() ;i++){
      GLfloat x = glm::distance(m_firstControlPoints[i],glm::vec3(v,0));
      if ( x < min){
          min=x;
          indx=i;
          id=1;
      }
   }
   for(size_t i=0; i < m_secondControlPoints.size() ;i++){
      GLfloat x = glm::distance(m_secondControlPoints[i],glm::vec3(v,0));
      if ( x < min){
          min=x;
          indx=i;
          id=2;
      }
   }
   if(min <= 0.6f)
       return {indx,id};
   else
       return {-1,-1};
}

void BSplineWindow::win2glcoord(glm::vec2 & v){
//translate according to pre defined size, TODO make this better
        v.x =( v.x / width()) * 20;
        v.y =( v.y / height())* 20;
        v.x -= 10;
        v.y -= 10;
        v.y *= -1;
}

void BSplineWindow::getCurveControlPoints(){
    if(!m_knots.size()){
        qDebug()<<"knots is empty"; return;
    }
    if(m_knots.size() < 2){
        qDebug() << "knots needs at least 2 points"; return;
    }
    if(m_knots.size() == 2){
        m_firstControlPoints = std::vector<glm::vec3>(1);
        m_secondControlPoints = std::vector<glm::vec3>(1);

        m_firstControlPoints[0]=(glm::vec3{
                                              (2 *m_knots[0].x + m_knots[1].x )/3,
                                              (2 *m_knots[0].y + m_knots[1].y )/3,
                                               0.0
                                          });
        m_secondControlPoints[0]=(glm::vec3{
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

std::vector<glm::vec3> BSplineWindow::updateControlPoints(std::vector<glm::vec3> & rhs){
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


GLfloat BSplineWindow::getBezier(GLfloat x, GLfloat k1, GLfloat c1, GLfloat c2, GLfloat k2){
    GLfloat s=1-x;
    GLfloat AB= k1*s + c1*x;
    GLfloat BC= c1*s + c2*x;
    GLfloat CD= c2*s + k2*x;
    GLfloat ABC= AB*s +BC*x;
    GLfloat BCD= BC*s +CD*x;

    return ABC*s+BCD*x;
}
