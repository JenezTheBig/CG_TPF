/*
 * Trabalho de Computacao Grafica
 * Projeto: Construccao de cena - Parque Urbano ao Entardecer
 *
 * Membros do Grupo:
 * - Rafael Carvalho Avidago Geraldo
 * - Matheus Duraes da Cunha Pereira
 *
 * Data: 29/11/2025
 *
 * Descricao:
 * Este programa cria uma cena 3D de um parque urbano ao entardecer com OpenGL.
 * A cena tem: banco, poste com luz, arvore, chao gramado e iluminacao direcional/pontual.
 *
 * Controles:
 * - Mouse: controla para onde a camera olha
 * - Setas: movem a camera na direcao do olhar
 * - Tecla 'A': liga/desliga luz pontual do poste
 * - Tecla 'S': liga/desliga luz direcional (sol)
 * - Tecla 'Esc': encerra
 */

#include <GL/glut.h>
#include <math.h>
#include <stdbool.h>

//Variaveis globais para gerenciar o centro da tela
int windowWidth = 800;
int windowHeight = 600;

// Variaveis de controle da camera (posicao inicial)
float cameraX = 0.0f;
float cameraY = 1.0f; // Altura para simular visao de pessoa
float cameraZ = 5.0f; // Distancia inicial da cena

//Variaveis de controle de iluminacao
bool luzPontualLigada = true; // poste comeca aceso
bool luzDirecionalLigada = true; // sol comeca ligado


//Variaveis globais para rotacao estilo FPS
float angleYaw = 0.0f;   // rotacao horizontal
float anglePitch = 0.0f; // rotacao vertical
int oldMouseX = 0;
int oldMouseY = 0;

// DECLARACOES PROTOTIPO
void configuraIluminacao();
void drawCena();


void mouseMotion(int x, int y) {
    //Calcula a diferenca do mouse
    int dx = x - oldMouseX;
    int dy = y - oldMouseY;
    
    // Atualiza angulos de rotacao 
    angleYaw += dx * 0.3f;     // rotacao horizontal
    anglePitch -= dy * 0.3f;   // rotacao vertical (invertida)
    
    //Limita o pitch para evitar inversao entre -89 e 89
    if (anglePitch > 89.0f) anglePitch = 89.0f;
    if (anglePitch < -89.0f) anglePitch = -89.0f;
    
    //Reposiciona mouse no centro da janela
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;
    
    if (x != centerX || y != centerY) {
        oldMouseX = centerX;
        oldMouseY = centerY;
        glutWarpPointer(centerX, centerY); //move mouse para o centro
    }
    
    glutPostRedisplay();
}

void init() {
    //Cor de fundo da cena
    glClearColor(0.2f, 0.2f, 0.3f, 1.0f); // azul escuro para entardecer

    // Habilita recursos basicos
    glEnable(GL_DEPTH_TEST); // ativa teste de profundidade
    glEnable(GL_LIGHTING);   // ativa iluminacao
    glEnable(GL_NORMALIZE);  // normaliza normais apos escalas

    // Ativa luzes padrao (configuracao detalhada em configuraIluminacao)
    glEnable(GL_LIGHT0); // luz direcional (sol)
    glEnable(GL_LIGHT1); // luz pontual (poste)
}

void reshape(int w, int h) {
    if (h == 0) h = 1; // evita divisao por zero
    float ratio = w * 1.0 / h;

    // guarda dimensoes da janela
    windowWidth = w;
    windowHeight = h;

    glMatrixMode(GL_PROJECTION); // modo projecao
    glLoadIdentity();

    // Projecao perspectiva: FOV 60, near 0.1, far 100
    gluPerspective(60.0f, ratio, 0.1f, 100.0f);

    glViewport(0, 0, w, h);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //Calcula direcao do olhar (estilo FPS)
    float yawRad = angleYaw * M_PI / 180.0f;
    float pitchRad = anglePitch * M_PI / 180.0f;
    
    //Direcao frontal baseada nos angulos
    float dirX = sin(yawRad) * cos(pitchRad);
    float dirY = sin(pitchRad);
    float dirZ = -cos(yawRad) * cos(pitchRad);
    
    // Ponto para o qual a camera olha
    float lookX = cameraX + dirX;
    float lookY = cameraY + dirY;
    float lookZ = cameraZ + dirZ;
    
    // Configura camera com gluLookAt
    gluLookAt(
        cameraX, cameraY, cameraZ,    // Posição da câmera
        lookX, lookY, lookZ,           // Ponto para o qual olha
        0.0f, 1.0f, 0.0f              // Vetor "up"
    );

    // Configurar Iluminação
    configuraIluminacao();

    // Desenho da Cena
    drawCena();

    glutSwapBuffers();
}

void desenhaChao() {
    // DESENHA O CHÃO (PRIMITIVA: GL_QUADS)
    // Material: Grama verde escura
    // Transformação: Quadrado plano no plano XZ (Y=0)
    
    GLfloat corGrama[] = {0.1f, 0.4f, 0.1f, 1.0f}; // Verde escuro
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corGrama);
    // Pequeno componente especular para evidenciar reflexo/local pool de luz
    GLfloat corGramaSpecular[] = {0.05f, 0.05f, 0.05f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, corGramaSpecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f);

    glBegin(GL_QUADS);
        // Normal aponta para cima (eixo Y positivo)
        glNormal3f(0.0f, 1.0f, 0.0f);

        // Vértices do chão (um plano grande no XZ)
        glVertex3f(-10.0f, 0.0f, -10.0f);
        glVertex3f(-10.0f, 0.0f,  10.0f);
        glVertex3f( 10.0f, 0.0f,  10.0f);
        glVertex3f( 10.0f, 0.0f, -10.0f);
    glEnd();
}

void desenhaAssento() {
    // Base do assento
    float largura = 1.8f;
    float espessura = 0.1f;
    float profundidade = 0.5f;

    glPushMatrix();
        // Escala: Redimensiona o cubo unitário para o tamanho do assento
        glScalef(largura, espessura, profundidade);
        glutSolidCube(1.0);
    glPopMatrix();
}

void desenhaPerna(float posX, float posZ) {
    // Perna do banco (um bloco fino)
    float tamanho = 0.7f;
    float espessura = 0.08f;

    glPushMatrix();
        // 1. Translação: Move a perna para a posição desejada
        glTranslatef(posX, tamanho / 2.0f, posZ);
        // 2. Escala: Redimensiona o cubo para a forma de uma perna
        glScalef(espessura, tamanho, espessura);
        glutSolidCube(1.0);
    glPopMatrix();
}

void desenhaCilindro(float raio, float altura) {
    //Desenha um cilindro usando primitivas basicas
    int numSegmentos = 16; // Numero de segmentos para formar o circulo
    float angulo;
    
    glPushMatrix();
        // Move para que a base do cilindro fique em Y=0
        glTranslatef(0.0f, altura / 2.0f, 0.0f);
        
        // Desenha o corpo do cilindro (lateral)
        glBegin(GL_QUAD_STRIP);
            glNormal3f(0.0f, 1.0f, 0.0f);
            for (int i = 0; i <= numSegmentos; i++) {
                angulo = 2.0f * M_PI * i / numSegmentos;
                float x = raio * cos(angulo);
                float z = raio * sin(angulo);
                
                // Normal apontando para fora
                glNormal3f(x / raio, 0.0f, z / raio);
                
                // Vertice superior
                glVertex3f(x, altura / 2.0f, z);
                // Vertice inferior
                glVertex3f(x, -altura / 2.0f, z);
            }
        glEnd();
        
        // Desenha a tampa superior
        glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, altura / 2.0f, 0.0f);
            for (int i = 0; i <= numSegmentos; i++) {
                angulo = 2.0f * M_PI * i / numSegmentos;
                glVertex3f(raio * cos(angulo), altura / 2.0f, raio * sin(angulo));
            }
        glEnd();
        
        // Desenha a base inferior
        glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(0.0f, -altura / 2.0f, 0.0f);
            for (int i = numSegmentos; i >= 0; i--) {
                angulo = 2.0f * M_PI * i / numSegmentos;
                glVertex3f(raio * cos(angulo), -altura / 2.0f, raio * sin(angulo));
            }
        glEnd();
    glPopMatrix();
}

void desenhaBanco() {
    // DESENHA O BANCO DE PRAÇA (PRIMITIVAS: Cubos escalados com glutSolidCube)
    // Composição: Assento, encosto e 4 pernas
    // Transformações hierárquicas: Translação, rotação e escala
    
    // Material do Banco (Madeira/Metal)
    GLfloat corBanco[] = {0.55f, 0.35f, 0.15f, 1.0f}; // Marrom
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corBanco);

    glPushMatrix(); // Salva a matriz de transformação
        // Posiciona o banco em (2, 0, -2) na cena
        glTranslatef(2.0f, 0.0f, -2.0f);
        glRotatef(30.0f, 0.0f, 1.0f, 0.0f); // Rotaciona o banco 30 graus

        float altAssento = 0.7f;
        float largura = 1.8f;
        float profundidade = 0.5f;

        // Desenhar Assento
        glPushMatrix();
            glTranslatef(0.0f, altAssento, 0.0f);
            desenhaAssento();
        glPopMatrix();

        // Desenhar Encosto
        float alturaEncosto = 0.8f;
        float espessuraEncosto = 0.1f;
        float espessuraAssento = 0.1f;
        
        glPushMatrix();
            float topoAssento = altAssento + espessuraAssento/2.0f;
            glTranslatef(0.0f, topoAssento + alturaEncosto/2.0f, -profundidade/2.0f - espessuraEncosto/2.0f);
            glScalef(largura, alturaEncosto, espessuraEncosto);
            glutSolidCube(1.0);
        glPopMatrix();

        // Desenhar as 4 Pernas (cantos do banco)
        float offset = (largura / 2.0f) - 0.1f;
        float offsetZ = (profundidade / 2.0f) - 0.1f;

        desenhaPerna(offset, offsetZ);      // Canto Frontal Direito
        desenhaPerna(-offset, offsetZ);     // Canto Frontal Esquerdo
        desenhaPerna(offset, -offsetZ);     // Canto Traseiro Direito
        desenhaPerna(-offset, -offsetZ);    // Canto Traseiro Esquerdo

    glPopMatrix(); // Restaura a matriz de transformação
}

void desenhaPoste() {
    // DESENHA O POSTE DE ILUMINAÇÃO (PRIMITIVAS: Cubo e esfera)
    // Composição: Base cilíndrica (feita com cubo escalado) e luminária (esfera)
    // Transformações: Translação e escala
    // Iluminação: Luz pontual posicionada no topo da luminária
    
    GLfloat corMetal[] = {0.3f, 0.3f, 0.3f, 1.0f}; // Cinza escuro para o poste
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corMetal);

    glPushMatrix(); // Salva a matriz de transformação
        // Posiciona o poste em (-3, 0, 0) na cena
        glTranslatef(-3.0f, 0.0f, 0.0f);

        // --- Base do Poste (cilindro fino e alto) ---
        // define medidas do poste
        float alturaPoste = 3.0f;
        float raioPoste = 0.08f;

        // desenha a base do poste como um cilindro escalado
        glPushMatrix();
            desenhaCilindro(raioPoste, alturaPoste);
        glPopMatrix();

        // --- Luminaria (esfera no topo do poste onde a luz esta) ---
        // Define material diferente se a luz do poste esta ligada (emissivo)
        GLfloat corLuminariaOff[] = {0.4f, 0.4f, 0.3f, 1.0f};
        GLfloat corLuminariaOn[]  = {1.0f, 0.9f, 0.6f, 1.0f};
        GLfloat emisOn[] = {0.8f, 0.7f, 0.3f, 1.0f};
        GLfloat emisOff[] = {0.0f, 0.0f, 0.0f, 1.0f};

        if (luzPontualLigada) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corLuminariaOn);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisOn);
        } else {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corLuminariaOff);
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisOff);
        }

        glPushMatrix();
            float raioLuminaria = 0.2f;
            glTranslatef(0.0f, alturaPoste + raioLuminaria/2.0f, 0.0f);
            glutSolidSphere(raioLuminaria, 16, 16);
        glPopMatrix();

        // Após desenhar, garante que emissao nao vaze para outros objetos
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisOff);

    glPopMatrix(); // Restaura a matriz de transformação
}



void desenhaArvore() {
    //DESENHA A ARVORE (PRIMITIVAS: cubo escalado para tronco e esfera para copa)
    // Composicao: tronco e copa
    // transformacoes hierarquicas: translacao e escala
    
    glPushMatrix();
        // Posiciona a árvore em (-5, 0, 5) na cena
        glTranslatef(-5.0f, 0.0f, 5.0f);

        // --- Tronco (Cilindro marrom) ---
        GLfloat corTronco[] = {0.4f, 0.2f, 0.0f, 1.0f}; // Marrom
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corTronco);

        // dimensoes do tronco
        float alturaTronco = 2.5f;
        float raioTronco = 0.2f;

        // desenha tronco usando cilindro helper
        glPushMatrix();
            desenhaCilindro(raioTronco, alturaTronco);
        glPopMatrix();

        // --- Copa (Esfera verde) ---
        GLfloat corCopa[] = {0.0f, 0.5f, 0.0f, 1.0f}; // Verde escuro
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corCopa);

        glPushMatrix();
            float raioCopa = 1.2f;
            glTranslatef(0.0f, alturaTronco + raioCopa * 0.5f, 0.0f);
            glutSolidSphere(raioCopa, 20, 20);
        glPopMatrix();

    glPopMatrix(); // Restaura a matriz de transformação
}

void configuraIluminacao() {
    // Esta função configura a iluminação mesmo quando as luzes estão desligadas
    // As luzes que estão desabilitadas não afetarão o cálculo de iluminação
    
    // -----------------------------------------------------
    // 1. LUZ AMBIENTE GERAL (Escuro de Entardecer)
    // -----------------------------------------------------
    // Define um nivel mais baixo de luz ambiente para aumentar contraste
    GLfloat luzAmbienteGlobal[] = {0.05f, 0.05f, 0.07f, 1.0f}; // mais escuro para destacar fontes locais
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbienteGlobal);

    // Habilita a atenuação de luz especular (mais realista)
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    // -----------------------------------------------------
    // 2. LUZ DIRECIONAL - SOL POENTE (GL_LIGHT0)
    // simula o por do sol
    // -----------------------------------------------------

    // cor do sol: laranja/amarelo
    GLfloat corSolDifusa[] = {0.8f, 0.4f, 0.1f, 1.0f};
    GLfloat corSolAmbiente[] = {0.2f, 0.1f, 0.05f, 1.0f};
    GLfloat corSolEspecular[] = {1.0f, 0.6f, 0.3f, 1.0f};

    // W=0.0 -> luz direcional
    GLfloat posSol[] = {3.0f, 3.0f, 3.0f, 0.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, corSolAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, corSolDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, corSolEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, posSol);

    // -----------------------------------------------------
    // 3. LUZ PONTUAL - POSTE (GL_LIGHT1)
    // simula a lampada no topo do poste
    // -----------------------------------------------------

    // cor do poste: branco quente (difusa) e posicao pontual
    GLfloat corPosteDifusa[] = {1.2f, 1.05f, 0.85f, 1.0f}; // levemente mais forte para iluminar o chao
    GLfloat corPosteAmbiente[] = {0.02f, 0.02f, 0.02f, 1.0f};
    GLfloat corPosteEspecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    // W=1.0 -> luz pontual
    GLfloat posPoste[] = {-3.0f, 3.1f, 0.0f, 1.0f};

    glLightfv(GL_LIGHT1, GL_AMBIENT, corPosteAmbiente);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, corPosteDifusa);
    glLightfv(GL_LIGHT1, GL_SPECULAR, corPosteEspecular);
    glLightfv(GL_LIGHT1, GL_POSITION, posPoste);

    // Atenuacao da luz pontual ajustada para melhor pool de luz no chao
    // diminuimos o termo quadratico e linear para ampliar o alcance perceptivel
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.8f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.03f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.005f);
}

void drawCena() {
    // Função que desenha todos os objetos da cena
    // Chamada a cada frame para renderizar a cena completa
    desenhaChao();      // Desenha o chão gramado
    desenhaBanco();     // Desenha o banco de praça
    desenhaPoste();     // Desenha o poste de iluminação
    desenhaArvore();    // Desenha a árvore
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27: // Tecla "Esc"
            exit(0);
            break;

        case 'a': // Tecla "a": liga/desliga a luz pontual do poste 
        case 'A':
            luzPontualLigada = !luzPontualLigada;
            if (luzPontualLigada)
                glEnable(GL_LIGHT1);
            else
                glDisable(GL_LIGHT1);
            break;

        case 's': // Tecla "s": liga/desliga a luz direcional (sol) 
        case 'S':
            luzDirecionalLigada = !luzDirecionalLigada;
            if (luzDirecionalLigada)
                glEnable(GL_LIGHT0);
            else
                glDisable(GL_LIGHT0);
            break;
    }
    glutPostRedisplay(); // Força o redesenho da cena para aplicar as mudanças
}

void specialKeys(int key, int x, int y) {
    float moveSpeed = 0.5f; // Velocidade de movimento
    
    // Calcular a direção frontal baseada na orientação atual (estilo FPS)
    float yawRad = angleYaw * M_PI / 180.0f;
    float pitchRad = anglePitch * M_PI / 180.0f;
    
    // Direção frontal (ignorando pitch para movimento horizontal)
    float dirX = sin(yawRad);
    float dirZ = -cos(yawRad);
    
    // Direção para a direita (perpendicular à frente)
    float rightX = cos(yawRad);
    float rightZ = sin(yawRad);
    
    // Direção vertical (não usada diretamente no movimento horizontal)
    
    switch (key) {
        case GLUT_KEY_UP:
            // Avança na direção para a qual está olhando
            cameraX += dirX * moveSpeed;
            cameraZ += dirZ * moveSpeed;
            break;
        case GLUT_KEY_DOWN:
            // Recua na direção oposta
            cameraX -= dirX * moveSpeed;
            cameraZ -= dirZ * moveSpeed;
            break;
        case GLUT_KEY_LEFT:
            // Move para a esquerda
            cameraX -= rightX * moveSpeed;
            cameraZ -= rightZ * moveSpeed;
            break;
        case GLUT_KEY_RIGHT:
            // Move para a direita
            cameraX += rightX * moveSpeed;
            cameraZ += rightZ * moveSpeed;
            break;
    }
    
    // Garante que a cena seja redesenhada com a nova posição
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Parque Urbano ao Entardecer");

    // Define as dimensões iniciais da janela
    windowWidth = 800;
    windowHeight = 600;

    init();

    // Esconde o cursor do mouse
    glutSetCursor(GLUT_CURSOR_NONE);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    
    // Vinculação de funções de movimentação de câmera:
    glutSpecialFunc(specialKeys);      // Para as setas do teclado
    // Usamos apenas movimento passivo do mouse para o modo FPS (sem botão)
    glutPassiveMotionFunc(mouseMotion); // Para o movimento do mouse sem botão
    
    // Posiciona o mouse no centro da tela inicialmente
    glutWarpPointer(windowWidth / 2, windowHeight / 2);
    oldMouseX = windowWidth / 2;
    oldMouseY = windowHeight / 2;
    
    glutMainLoop();

    return 0;
}