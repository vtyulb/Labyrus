#include "model.h"

Model::Model(QString s) {
    FILE *fin = fopen(s.toAscii(), "r");

    char buf[255];
    fscanf(fin, "%s", buf);
    if (buf != QString("#stdupid3dformat\n"))
        qDebug() << "model data is not correct";

    fscanf(fin, "%d", &n);
    r = new int*[n];
    for (int i = 0; i < n; i++) {
        r[i] = new int[3];
        for (int j = 0; j < 3; j++)
            fscanf(fin, "%d", &r[i][j]);
    }

    fclose(fin);
}

void Model::draw(double k, double x, double y, double z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            for (int l = 0; l < n; l++) {
                glVertex3f(r[i][0] * k, r[i][1] * k, r[i][2] * k);
                glTexCoord2d(0, 0);
                glVertex3f(r[j][0] * k, r[j][1] * k, r[j][2] * k);
                glTexCoord2d(0, 1);
                glVertex3f(r[l][0] * k, r[l][1] * k, r[l][2] * k);
                glTexCoord2d(1, 0);
            }

    glEnd();

    glPopMatrix();
}
