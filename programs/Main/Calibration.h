


/* Liegt der Arduino gerade auf dem Tisch und wird nicht bewegt, muss die Z-Achse 1G (Erdbeschleunigung) ausgeben und der Rest muss 0 sein.
   Das ist nicht wirklich so und wird mit diesen Werten kalibriert: */
const float ACC_X_OFFSET = +0.0231;
const float ACC_Y_OFFSET = +0.0105;
const float ACC_Z_OFFSET = -0.0102;

/* Wird der Arduino nicht rotiert, müssen alle Winkelbeschleunigungen 0 sein.
   Das ist nicht wirklich so und wird mit diesen Werten kalibriert: */
const float GYRO_X_OFFSET = -0.43;
const float GYRO_Y_OFFSET = +0.67;
const float GYRO_Z_OFFSET = +0.31;
