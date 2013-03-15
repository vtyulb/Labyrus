#include "drawgl.h"
#include <QtOpenGL/QGLWidget>
#include <GL/glu.h>

DrawGl::DrawGl(QApplication *app, QString skin, QWidget *parent) :
    QGLWidget(parent)
{
    application = app;
    skinPath = skin;
    enteringText = false;
    xRot = -90;
    yRot = 0;
    zRot = 0;
    perspective = 45;

    animZRot = 0;
    fps = 0;
    oldFps = 0;

    nSca = 1;

    xtra = ytra = 0;
    ztra = -wallHeight / 2;
    startingGame = false;

    hudFont = QFont("FreeSans", 15, 20, true);

//    t = new QTimer;
    timeFPS = new QTimer;
//    t->setInterval(3);
    timeFPS->setInterval(1000);
    startAfter = 666;

//    QObject::connect(t, SIGNAL(timeout()), this, SLOT(onx()));
    QObject::connect(timeFPS, SIGNAL(timeout()), this, SLOT(drawFPS()));
    this->setFocus();
//    t->start();
    timeFPS->start();


    firstMouseMove = true;
    botActive = false;
    QCursor::setPos(width() / 2, height() / 2);
    setCursor(QCursor(Qt::BlankCursor));
    setMouseTracking(true);
    mousePressed = false;

    compass = new QPixmap(skinPath + "/compass.png");
    needRefreshCursor = true;
    mouseSensitivity = 1 / 3.0;
}

void DrawGl::initializeGL() {
    qglClearColor(Qt::gray);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    textures[0] = bindTexture(QPixmap(skinPath + "/defaultWall.jpg"), GL_TEXTURE_2D);
    textures[1] = bindTexture(QPixmap(skinPath + "/shortWall.jpg"), GL_TEXTURE_2D);
    textures[2] = bindTexture(QPixmap(skinPath + "/roof.jpg"), GL_TEXTURE_2D);
    textures[3] = bindTexture(QPixmap(skinPath + "/floor.jpg"), GL_TEXTURE_2D);
    textures[4] = bindTexture(QPixmap(skinPath + "/compass.png"), GL_TEXTURE_2D);
    textures[5] = bindTexture(QPixmap(skinPath + "/sky.jpg"), GL_TEXTURE_2D);
    textures[6] = bindTexture(QPixmap(skinPath + "/model.jpg"), GL_TEXTURE_2D);
    textures[7] = bindTexture(QPixmap(skinPath + "/realRoof.jpg"), GL_TEXTURE_2D);


    I = new Model(skinPath + "/simple.s3d");

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_2D);

    glFrontFace(GL_CCW);
    glPointSize(10);
}

void DrawGl::resizeGL(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, (GLint)w, (GLint)h);
    gluPerspective(perspective, w / (double)h, 0.001, 1000000.0);
    if (isFullScreen())
        QCursor::setPos(w / 2, h / 2);

    k = 1.0 / sizeView;
    f = k / 10;
    needRefreshCursor = true;
}

void DrawGl::paintGL() {
    if (a->radiation) {
        a->radiation = false;
        if (perspective < 150)
            perspective++;
           this->resizeGL(this->width(), this->height());
    }
    fps++;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotatef(a->yAngle, 1.0f, 0.0f, 0.0f);
    glRotatef(yRot, 0.0f, 1.0f, 0.0f);
    glRotatef(a->angle, 0.0f, 0.0f, 1.0f);

    nSca = a->n / 2.0;
    glScalef(nSca, nSca, nSca);
    glTranslatef(-a->coord.x / sizeView, -a->coord.y / sizeView, ztra - a->coord.h / sizeView);

//    drawAxis();
    enableLight();
    drawSkyBox();
    drawMaze();
    drawHeroes();
    drawCompass();
    drawHUD();
}

void DrawGl::drawAxis() {
    glLineWidth(3.0f);

    glBegin(GL_LINES);
        qglColor(Qt::red);

        glVertex3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, 0.0f, 0.0f);

        qglColor(Qt::green);

        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -1.0f, 0.0f);

        qglColor(Qt::blue);

        glVertex3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -1.0f);
    glEnd();

    renderText(0.0, 0.0, 1.0, QString::number(1));
    renderText(0.0, 1.0, 0.0, QString::number(1));
    renderText(1.0, 0.0, 0.0, QString::number(1));
}

void DrawGl::enableLight() {
        float dir[3] = {0, 0, -1};
//    GLfloat pos[4] = {(float)(a->coord.x()), (float)(a->coord.y()), -wallHeight / 2, 1.0f};
        GLfloat pos[4] = {0.5, 0.5, 0.01, 1.0f};
        GLfloat color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat mat_specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        GLfloat ambientLight[] = {0.1f, 0.1f, 0.1f, 0.8f};


        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

        glLightfv(GL_LIGHT0, GL_POSITION, pos);
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);
        glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, color);
        glEnable(GL_LIGHT0);



        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_specular);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void DrawGl::drawSkyBox() {
    loadTexture(textures[5]);
    glBegin(GL_QUADS);
   /*     for (int i = 0; i < a->n; i++)
            for (int j = 0; j < a->n; j++) {
                glVertex3f((i + 1) / sizeView, j / sizeView, wallHeight);
                glTexCoord2d(1, 1);
                glVertex3f(i / sizeView, j / sizeView, wallHeight);
                glTexCoord2d(0, 1);
                glVertex3f(i / sizeView, (j + 1) / sizeView, wallHeight);
                glTexCoord2d(0, 0);
                glVertex3f((i + 1) / sizeView, (j + 1) / sizeView, wallHeight);
                glTexCoord2d(1, 0);
            }*/
        glVertex3f(50, -50, 10);
        glTexCoord2d(1, 1);
        glVertex3f(-50, -50, 10);
        glTexCoord2d(0, 1);
        glVertex3f(-50, 50, 10);
        glTexCoord2d(0, 0);
        glVertex3f(50, 50, 10);
        glTexCoord2d(1, 0);
    glEnd();
}

void DrawGl::drawQuad(double x1, double y1, double x2, double y2, double h, double height) {
    glVertex3f(x1, y1, h + height);
    glTexCoord2d(0, 0);
    glVertex3f(x1, y1, h);
    glTexCoord2d(1, 0);
    glVertex3f(x2, y2, h);
    glTexCoord2d(1, 1);
    glVertex3f(x2, y2, h + height);
    glTexCoord2d(0, 1);
}

void DrawGl::drawRoofPart(double x, double y, double h, int type, bool b) {
    if (type == 1) {
        if (b) {
            glVertex3f(x - f, y + k, h + eps);
            glTexCoord2d(0, 1);
            glVertex3f(x - f, y, h + eps);
            glTexCoord2d(0, 0);
            glVertex3f(x + f, y, h + eps);
            glTexCoord2d(1, 0);
            glVertex3f(x + f, y + k, h + eps);
            glTexCoord2d(1, 1);
        } else {
            glVertex3f(x + f, y, h + eps);
            glTexCoord2d(1, 0);
            glVertex3f(x - f, y, h + eps);
            glTexCoord2d(0, 0);
            glVertex3f(x - f, y + k, h + eps);
            glTexCoord2d(0, 1);
            glVertex3f(x + f, y + k, h + eps);
            glTexCoord2d(1, 1);
        }
    } else if (type == 0) {
        if (b) {
            glVertex3f(x + k, y + f, h + eps);
            glTexCoord2d(1, 0);
            glVertex3f(x, y + f, h + eps);
            glTexCoord2d(0, 0);
            glVertex3f(x, y - f, h + eps);
            glTexCoord2d(0, 1);
            glVertex3f(x + k, y - f, h + eps);
            glTexCoord2d(1, 1);
        } else {
            glVertex3f(x, y - f, h + eps);
            glTexCoord2d(0, 1);
            glVertex3f(x, y + f, h + eps);
            glTexCoord2d(0, 0);
            glVertex3f(x + k, y + f, h + eps);
            glTexCoord2d(1, 0);
            glVertex3f(x + k, y - f, h + eps);
            glTexCoord2d(1, 1);
        }
    }
}

void DrawGl::drawMaze() {
    qglColor(Qt::white);
    loadTexture(textures[0]);
    glBegin(GL_QUADS);
        for (int i = 0; i < a->m; i++) {
            double x = a->walls[i][0] * k;
            double y = a->walls[i][1] * k;
            double h = a->walls[i][2] * wallHeight;
            if (a->walls[i][3] == 0) {
                drawQuad(x, y - f, x + k, y - f, h, wallHeight);
                drawQuad(x + k, y + f, x, y + f, h, wallHeight);
            } else if (a->walls[i][3] == 1) {
                drawQuad(x + f, y, x + f, y + k, h, wallHeight);
                drawQuad(x - f, y + k, x - f, y, h, wallHeight);
            } else {
                drawQuad(x, y, x + k, y, h - f, f * 2);
                drawQuad(x, y + k, x, y, h - f, f * 2);
                drawQuad(x + k, y, x + k, y + k, h - f, f * 2);
                drawQuad(x + k, y + k, x, y + k, h - f, f * 2);
            }
        }
    glEnd();

    loadTexture(textures[1]);
    glBegin(GL_QUADS);
        for (int i = 0; i < a->m; i++) {
            double x = a->walls[i][0] * k;
            double y = a->walls[i][1] * k;
            double h = a->walls[i][2] * wallHeight;
            if (a->walls[i][3] == 0) {
                drawQuad(x, y + f, x, y - f, h, wallHeight);
                drawQuad(x + k, y - f, x + k, y + f, h, wallHeight);
            } else if (a->walls[i][3] == 1) {
                drawQuad(x - f, y, x + f, y, h, wallHeight);
                drawQuad(x + f, y + k, x - f, y + k, h, wallHeight);
            }
        }
    glEnd();

    loadTexture(textures[2]);
    glBegin(GL_QUADS);
        for (int i = 0; i < a->m; i++) {
            double x = a->walls[i][0] * k;
            double y = a->walls[i][1] * k;
            double h = a->walls[i][2] * wallHeight;
            drawRoofPart(x, y, h, a->walls[i][3], false);
            drawRoofPart(x, y, h + wallHeight, a->walls[i][3], true);
        }
    glEnd();

    loadTexture(textures[3]);
    glBegin(GL_QUADS);
        for (int i = 0; i < a->n; i++)
            for (int j = 0; j < a->n; j++) {
                glVertex3f(i * k, (j + 1) * k, -eps);
                glTexCoord2d(1, 1);
                glVertex3f(i * k, j * k, -eps);
                glTexCoord2d(0, 1);
                glVertex3f((i + 1) * k, j * k, -eps);
                glTexCoord2d(0, 0);
                glVertex3f((i + 1) * k, (j + 1) * k, -eps);
                glTexCoord2d(1, 0);
            }

    glEnd();

    loadTexture(textures[realRoof]);
    glBegin(GL_QUADS);
        for (int i = 0; i < a->m; i++)
            if ((a->walls[i][3] == 2) && (a->h != 1))
                drawFloorPoint(a->walls[i][0], a->walls[i][1], a->walls[i][2], false);

    glEnd();

    loadTexture(textures[Floor]);
    glBegin(GL_QUADS);
        for (int i = 0; i < a->m; i++)
            if ((a->walls[i][3] == 2) && (a->h != 1))
                drawFloorPoint(a->walls[i][0], a->walls[i][1], a->walls[i][2], true);
    glEnd();


    qglColor(Qt::blue);

    glLineWidth(1);
    drawText(f + eps, f + eps, wallHeight / 2, false, true, QString::fromLocal8Bit("Добро Пыжаловать!!!"));
//    drawText(k - 2 * f, 2 * f, wallHeight / 2, true, false, QString("Welcome to SuperMaze on x"));

//    if (ztra < -wallHeight)
//        I->draw(1 / sizeView / 10, a->coord.x() * k, a->coord.y() * k, wallHeight / 3);
    if (startingGame)
        renderText(this->width() / 2 - 100, this->height() / 2, QString("Starting after ") + QString::number((3000 - startAfter) / 1000) + QString(" seconds"), hudFont);
}

void DrawGl::onx() {
//    xRot += 1;
    zRot += 1;
    repaint();
}

void DrawGl::drawFPS() {
    if (rand() % 100 == 0) {
        qDebug() << "current fps:" << fps;
        qDebug() << "current opengl error:" << glGetError();
    }
    oldFps = fps;
    fps = 0;
}

void DrawGl::keyPressEvent(QKeyEvent *event) {
    if (event->key() == (Qt::Key_Enter xor 1)) {
        enteringText = !enteringText;
        if (!enteringText)
            processText();

        currentText = "";
        return;
    } else if (enteringText)
        if (event->key() == Qt::Key_Backspace)
            currentText = currentText.left(currentText.length() - 1);
        else if (event->key() == Qt::Key_Escape) {
            currentText = "";
            enteringText = false;
        } else
            currentText += event->text();
    else {
        legacy->keyPressEvent(event);
    }

    event->accept();
}

void DrawGl::keyReleaseEvent(QKeyEvent *event) {
    legacy->keyReleaseEvent(event);
}

void DrawGl::mousePressEvent(QMouseEvent *event) {
    mousePressed = true;
    event->accept();
}

void DrawGl::mouseReleaseEvent(QMouseEvent *event) {
    mousePressed = false;
    event->accept();
}

/*void DrawGl::wheelEvent(QWheelEvent *event) {
    if (event->delta() > 0)
        a->fgup();
    else
        a->fgdown();
}*/

void DrawGl::mouseMoveEvent(QMouseEvent *event) {
    if (botActive || (!this->isFullScreen()) || (legacy->thread->currentTime < 100))
        return;

    double x = (event->x() - width() / 2) * mouseSensitivity;
    double y = (event->y() - height() / 2)  * mouseSensitivity;

    QCursor::setPos(width() / 2, height() / 2);

    a->angle += x;
    a->yAngle += y;
    a->checkAngles();
//    qDebug() << event->x() - width() / 2 - this->pos().x();
}


void DrawGl::loadTexture(GLuint a) {
    glBindTexture(GL_TEXTURE_2D, a);
}

void DrawGl::drawText(double x, double y, double z, bool xForwarding, bool yForwarding, QString s) {
    QPainterPath a;
    a.addText(0, 20, QFont("FreeSans", 10), s);
    QList<QPolygonF> l = a.toSubpathPolygons();
        for (QList<QPolygonF>::iterator i = l.begin(); i != l.end(); i++) {
        glBegin(GL_LINE_STRIP);
            for (QPolygonF::iterator j = (*i).begin(); j != (*i).end(); j++) {
                qglColor(Qt::red);
                glVertex3f(j->rx() * 0.0005f * xForwarding + x, j->rx() * 0.0005f * yForwarding + y, -j->ry() * 0.0005f + z);
            }
        glEnd();
    }
}

void DrawGl::processText() {
    if (currentText == "")
        return;
    emit runCommand("I\n" + currentText);
    currentText = currentText.toUpper();
    if (currentText == "EXIT") {
        legacy->legalStop();
    } else if (currentText == "BOT") {
        legacy->startBot();
    } else if (currentText == "STOP") {
        legacy->stopBot = true;
    } else if (currentText == "PING") {
        emit runCommand("p\n");
        a->pingTime = new QTime;
        a->pingTime->start();
    } else if (currentText == "HELP") {
        a->messages->addMessage("Possible commands: help bot stop ping exit...");
    }

    qDebug() << currentText << "processed";
}

void DrawGl::begin2d() {
  glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, this->width(), 0, this->height());
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void DrawGl::end2d() {
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glEnable(GL_DEPTH_TEST);
}

QPixmap DrawGl::generateCompass(double angle) {
    QPixmap res(400, 400);
    res.fill(Qt::transparent);

    QPainter p;
    p.begin(&res);

    QTransform transform;
    transform.translate(199, 199);
    transform.rotate(angle);
    p.setTransform(transform);

    p.drawPixmap(-200, -200, QPixmap(skinPath + "/compass.png"));
    p.end();
    return res;
}

void DrawGl::drawFloorPoint(double x, double y, double h, bool b) {
    if (b) {
        glVertex3f((x + 1) * k, (y + 1) * k, h * k + k / 10);
        glTexCoord2d(1, 1);
        glVertex3f(x * k, (y + 1) * k, h * k + k / 10);
        glTexCoord2d(0, 1);
        glVertex3f(x * k, y * k, h * k + k / 10);
        glTexCoord2d(0, 0);
        glVertex3f((x + 1) * k, y * k, h * k + k / 10);
        glTexCoord2d(1, 0);
    } else {
        glVertex3f((x + 1) * k, (y + 1) * k, h * k - k / 10);
        glTexCoord2d(1, 1);
        glVertex3f((x + 1) * k, y * k, h * k - k / 10);
        glTexCoord2d(1, 0);
        glVertex3f(x * k, y * k, h * k - k / 10);
        glTexCoord2d(0, 0);
        glVertex3f(x * k, (y + 1) * k, h * k - k / 10);
        glTexCoord2d(0, 1);
    }
}

void DrawGl::drawHeroes() {
    qglColor(Qt::black);
    loadTexture(textures[model]);
    for (int i = 0; i < a->otherHeroes; i++)
        if ((a->heroes[i].x != -1) || (a->heroes[i].y) != -1) {
            qglColor(QColor(0, a->otherAlive[i] * 200, 0));

            renderText(a->heroes[i].x * k, a->heroes[i].y * k, a->heroes[i].h * k + 3 * f, a->heroNames[i], hudFont);
            I->draw(1 / sizeView / 10, a->heroes[i].x * k, a->heroes[i].y * k, a->heroes[i].h * k);
        }
}

void DrawGl::drawCompass() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    deleteTexture(textures[4]);
    textures[4] = bindTexture(generateCompass(a->angle), GL_TEXTURE_2D);
    begin2d();
    loadTexture(textures[4]);
    glBegin(GL_QUADS);
        glVertex2d(this->width() - 200, 0);
        glTexCoord2d(0, 0);
        glVertex2d(this->width(), 0);
        glTexCoord2d(0, 1);
        glVertex2d(this->width(), 200);
        glTexCoord2d(1, 1);
        glVertex2d(this->width() - 200, 200);
        glTexCoord2d(1, 0);
    glEnd();
    end2d();

    glDisable(GL_BLEND);
}

void DrawGl::drawHUD() {
    qglColor(Qt::blue);
    renderText(5, 15, tr("From start game: ") + QString::number(legacy->thread->fromStartOfGame.elapsed() / 1000) + QString("s"), hudFont);
    renderText(5, this->height() - 20, tr("Alive status: ") + QString::number(a->alive), hudFont);
    renderText(5, this->height() - 40, tr("patrons: ") + QString::number(a->patrons), hudFont);
    renderText(5, this->height() - 60, tr("walls: ") + QString::number(a->wall), hudFont);
    renderText(5, this->height() - 80, tr("destroy: ") + QString::number(a->destroy), hudFont);
    renderText(5, this->height() - 100, tr("Floor number: ") + QString::number(a->getFloor()), hudFont);
    renderText(this->width() - 60, 10, QString("FPS: ") + QString::number(oldFps));

    qglColor(Qt::red);
    if (enteringText)
        renderText(5, this->height() - 120, "-" + currentText, hudFont);

    loadTexture(textures[2]);
    QList<QString> list = a->messages->getMessages();
    for (int i = 0; i < list.size(); i++)
        renderText(5, this->height() - 120 - 20 * (list.size() - i), list[i], hudFont);
}
