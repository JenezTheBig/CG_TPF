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

//Variaveis globais para gerenciar o tamanho da tela 
int windowWidth = 800;
int windowHeight = 600;

// Variaveis de controle da camera (posicao inicial)
float cameraX = 0.0f;
float cameraY = 1.0f; // Altura para simular visao de pessoa
float cameraZ = 5.0f; // Distancia inicial da cena

//Variaveis de controle de iluminacao
bool luzPontualLigada = true; // poste comeca aceso
bool luzDirecionalLigada = true; // sol comeca ligado


//Variaveis globais para rotacao (para implementacao do stilo fps)
float angleYaw = 0.0f;   // rotacao horizontal (movimento de mouse esquerda / direita)
float anglePitch = 0.0f; // rotacao vertical (movimento de mouse cima/ baixo)
int oldMouseX = 0; // salva posicao anterior do mouse nos 
int oldMouseY = 0;

// DECLARACOES PROTOTIPO
void configuraIluminacao();
void drawCena();

// define para onde a camera deve apontar
void mouseMotion(int x, int y) {
    //Calcula a diferenca do mouse
    int dx = x - oldMouseX; // x e y sao coordenadas atuais do mouse e oldMouseX/Y sao as anteriores.
    int dy = y - oldMouseY; // o calculo registra o quanto e para onde o mouse foi movido, dx positivo moveu para a direito
                            // dy positivo moveu para baixo

    // Atualiza angulos de rotacao (estilo FPS)
    angleYaw += dx * 0.3f;     // adiciona rotacao horizontal ao angulo horizontal da camera e 0.3 e a sensibilidade
    anglePitch -= dy * 0.3f;   // rotacao vertical (invertida para que arraste para frente olhe para cima)
    
    //Limita o pitch para evitar inversao entre -89 e 89
    if (anglePitch > 89.0f) anglePitch = 89.0f; // caso o angulo de visualizacao exceda 89 , trava em 89
    if (anglePitch < -89.0f) anglePitch = -89.0f;
    
    //Reposiciona mouse no centro da janela
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;
    
    if (x != centerX || y != centerY) { // verifica se o mouse saiu do centro, o movimento e calculado
        oldMouseX = centerX;    // salva o movimento e forca o mouse a voltar para o centro fisico da janela
        oldMouseY = centerY;// oldMouseX/Y voltam a ser o centro criando ilusao de movimento infinito
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

//funcao de janela 
void reshape(int w, int h) {
    if (h == 0) h = 1; // evita divisao por zero
    float ratio = w * 1.0 / h;

    // guarda dimensoes da janela
    windowWidth = w;
    windowHeight = h;

    glMatrixMode(GL_PROJECTION); // modo projecao
    glLoadIdentity(); // reseta matriz para estado original

    // Projecao perspectiva: FOV 60, near 0.1, far 100
    gluPerspective(60.0f, ratio, 0.1f, 100.0f);

    glViewport(0, 0, w, h);//diz que a area comeca em 0,0 e vai ate w e h
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //pinta a tela com a cor de fundo (azul escuro) e reseta info de profundidade

    glMatrixMode(GL_MODELVIEW); //modo de view
    glLoadIdentity();   //coloca cursor no ponto 0 0 0 

    //Calcula direcao do olhar (estilo FPS)
    float yawRad = angleYaw * M_PI / 180.0f;
    float pitchRad = anglePitch * M_PI / 180.0f;
    
    //Direcao frontal baseada nos angulos
    // converte graus para radianos nas funcoes sin e cos
    float dirX = sin(yawRad) * cos(pitchRad); // calculam vetor unitario que aoponta para onde a camera olha
    float dirY = sin(pitchRad);
    float dirZ = -cos(yawRad) * cos(pitchRad); // negatio pois a camera aponta para Z negativo naturalmente
    
    // Ponto para o qual a camera olha
    float lookX = cameraX + dirX; //pega a posica de onde voce esta e soma a direcao do olhar
    float lookY = cameraY + dirY;   // define ponto virtual 1 metro a frente (onde a camera esta mirando)
    float lookZ = cameraZ + dirZ;
    
    // Configura camera com gluLookAt
    gluLookAt( // pega todos os dados e move o mundo interio na direcao oposta.
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
    // DESENHA O CHÃO 
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
    // Define medida da caixa que sera criada
    float largura = 1.8f;
    float espessura = 0.1f;
    float profundidade = 0.5f;

    glPushMatrix();
        // Escala: Redimensiona o cubo unitário para o tamanho do assento
        glScalef(largura, espessura, profundidade); //passa os parametros de escala que serao multiplicados
        glutSolidCube(1.0); // cria um cubo 1x1x1
    glPopMatrix();
}

void desenhaPerna(float posX, float posZ) {
    // Perna do banco (um bloco fino)
    float tamanho = 0.7f;
    float espessura = 0.08f;

    glPushMatrix();
        // Move a perna para a posição desejada
        glTranslatef(posX, tamanho / 2.0f, posZ);
        // Escala: Redimensiona o cubo para a forma de uma perna
        glScalef(espessura, tamanho, espessura);
        glutSolidCube(1.0);
    glPopMatrix();
}

void desenhaCilindro(float raio, float altura) {
    //Desenha um cilindro
    int numSegmentos = 16; // Numero de segmentos para formar o circulo
    float angulo;
    
    glPushMatrix();
        // Move para que a base do cilindro fique em Y=0
        glTranslatef(0.0f, altura / 2.0f, 0.0f);
        
        // Desenha o corpo do cilindro 
        glBegin(GL_QUAD_STRIP); // dois pontos sao definidos, depois mais dois e quad strip os conecata
            glNormal3f(0.0f, 1.0f, 0.0f); // o par anterior com o novo
            for (int i = 0; i <= numSegmentos; i++) {
                angulo = 2.0f * M_PI * i / numSegmentos;
                float x = raio * cos(angulo); //transforma os angulos em posicao no circulo 
                float z = raio * sin(angulo);
                
                // Normal apontando para fora, pois na lateral do cilindro a luz deve refletir como se fosse arredondado.
                glNormal3f(x / raio, 0.0f, z / raio);
                
                // Vertice superior
                glVertex3f(x, altura / 2.0f, z);
                // Vertice inferior
                glVertex3f(x, -altura / 2.0f, z);
            }
        glEnd();
        
        // Desenha a tampa superior
        glBegin(GL_TRIANGLE_FAN);           // Leque de triangulos o centro e definido primeiro depois a cada ponto
            glNormal3f(0.0f, 1.0f, 0.0f);   // desenhado na porda, um triangulo  e criado e conectado ao centro
            glVertex3f(0.0f, altura / 2.0f, 0.0f);  //nomral fixa porque e plana e aponta para cima
            for (int i = 0; i <= numSegmentos; i++) {
                angulo = 2.0f * M_PI * i / numSegmentos;
                glVertex3f(raio * cos(angulo), altura / 2.0f, raio * sin(angulo));
            }
        glEnd();
        
        // Desenha a base inferior
        glBegin(GL_TRIANGLE_FAN);
            glNormal3f(0.0f, -1.0f, 0.0f);
            glVertex3f(0.0f, -altura / 2.0f, 0.0f);
            for (int i = numSegmentos; i >= 0; i--) { //i -- para inverter a ordem dos pontos e manter a face visivel virada para baixo
                angulo = 2.0f * M_PI * i / numSegmentos;
                glVertex3f(raio * cos(angulo), -altura / 2.0f, raio * sin(angulo));
            }
        glEnd();
    glPopMatrix();
}

void desenhaBanco() {
    // DESENHA O BANCO DE PRAÇA
    // Composição: Assento, encosto e 4 pernas
    // Transformações hierárquicas: Translação, rotação e escala

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
            glTranslatef(0.0f, altAssento, 0.0f); // ergue a altura do banco para 0.7
            desenhaAssento();
        glPopMatrix();

        // Desenhar Encosto
        float alturaEncosto = 0.8f;
        float espessuraEncosto = 0.1f;
        float espessuraAssento = 0.1f;
        
        glPushMatrix();
            float topoAssento = altAssento + espessuraAssento/2.0f;
        // a altura e a altura do acento+ metade da altura do encosto
        // para colocar o encosto, devemos empurralo para tras. Movemos para a vorda de tras (-profundidade/2)
        // e subtraimos a espessura do proprio encosto
            glTranslatef(0.0f, topoAssento + alturaEncosto/2.0f, -profundidade/2.0f - espessuraEncosto/2.0f);
            glScalef(largura, alturaEncosto, espessuraEncosto);
            glutSolidCube(1.0);
        glPopMatrix();

        // Desenhar as 4 Pernas (cantos do banco)
        float offset = (largura / 2.0f) - 0.1f;         // espelhamento das cordenadas, a subtracao por 1 evitta
        float offsetZ = (profundidade / 2.0f) - 0.1f;   // Que a perna fique para fora

        desenhaPerna(offset, offsetZ);      // Canto Frontal Direito
        desenhaPerna(-offset, offsetZ);     // Canto Frontal Esquerdo
        desenhaPerna(offset, -offsetZ);     // Canto Traseiro Direito
        desenhaPerna(-offset, -offsetZ);    // Canto Traseiro Esquerdo

    glPopMatrix(); // Restaura a matriz de transformação
}

void desenhaPoste() {
    // DESENHA O POSTE DE ILUMINAÇÃO 
    // Transformações: Translação e escala
    // Iluminação: Luz pontual posicionada no topo da luminária
    
    GLfloat corMetal[] = {0.3f, 0.3f, 0.3f, 1.0f}; // Cinza escuro para o poste
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corMetal);

    glPushMatrix();
        // Posiciona o poste em 
        glTranslatef(-3.0f, 0.0f, 0.0f);

        // --- Base do Poste (cilindro fino e alto) ---
        // define medidas do poste
        float alturaPoste = 3.0f;
        float raioPoste = 0.08f;

        // desenha a base do poste como um cilindro
        glPushMatrix();
            desenhaCilindro(raioPoste, alturaPoste);
        glPopMatrix();

        // --- Luminaria ---
        // Define material diferente se a luz do poste esta ligada (emissivo)
        GLfloat corLuminariaOff[] = {0.4f, 0.4f, 0.3f, 1.0f}; // Cinza claro quando desligada
        GLfloat corLuminariaOn[]  = {1.0f, 0.9f, 0.6f, 1.0f}; // Amarelo quente quando ligada
        GLfloat emisOn[] = {0.8f, 0.7f, 0.3f, 1.0f}; // Emissão amarela quando ligada
        GLfloat emisOff[] = {0.0f, 0.0f, 0.0f, 1.0f}; // Sem emissão quando desligada

        if (luzPontualLigada) {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corLuminariaOn); // amarelo claro quando ligada 
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisOn);                      // define emissao para amarelo forte
        } else {
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corLuminariaOff); // amarelo escuro para desligada
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisOff);                     // sem emissao
        }

        glPushMatrix();
            float raioLuminaria = 0.2f; //raio da bola
            glTranslatef(0.0f, alturaPoste + raioLuminaria/2.0f, 0.0f); //poiciona a bola no final do corpo do poste
            glutSolidSphere(raioLuminaria, 16, 16); //desenha a esfera
        glPopMatrix();

        // Após desenhar, garante que emissao nao vaze para outros objetos
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emisOff);

    glPopMatrix(); // Restaura a matriz de transformação
}



void desenhaArvore() {
    //DESENHA A ARVORE 
    // Composicao: tronco e copa
    // transformacoes: translacao e escala
    
    glPushMatrix();
        // Posiciona a árvore em (-5, 0, 5) na cena
        glTranslatef(-5.0f, 0.0f, 5.0f);

        // --- Tronco (Cilindro marrom) ---
        GLfloat corTronco[] = {0.4f, 0.2f, 0.0f, 1.0f}; // Marrom
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corTronco);

        // dimensoes do tronco
        float alturaTronco = 2.5f;
        float raioTronco = 0.2f;

        // desenha tronco usando cilindro 
        glPushMatrix();
            desenhaCilindro(raioTronco, alturaTronco);
        glPopMatrix();

        // --- Copa (Esfera verde) ---
        GLfloat corCopa[] = {0.0f, 0.5f, 0.0f, 1.0f}; // Verde escuro
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, corCopa);

        glPushMatrix();
            float raioCopa = 1.2f;
            glTranslatef(0.0f, alturaTronco + raioCopa * 0.5f, 0.0f); //poiciona a copa sobreposta ao final tronco
            glutSolidSphere(raioCopa, 20, 20);          // desenha a esfera
        glPopMatrix();

    glPopMatrix(); // Restaura a matriz de transformação
}

void configuraIluminacao() {
    // Esta função configura a iluminação mesmo quando as luzes estão desligadas

    // --- LUZ AMBIENTE GERAL (Escuro de Entardecer) ---

    // Define um nivel mais baixo de luz para que ambiente seja viivel, nao 100% preto
    GLfloat luzAmbienteGlobal[] = {0.05f, 0.05f, 0.07f, 1.0f}; // tom azulado escuro
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbienteGlobal);

    // Habilita a atenuação de luz especular (mais realista)
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);


    // --- LUZ DIRECIONAL - SOL POENTE (GL_LIGHT0) ---
    // simula o por do sol

    // cor do sol: laranja/amarelo
    GLfloat corSolDifusa[] = {0.8f, 0.4f, 0.1f, 1.0f};
    GLfloat corSolAmbiente[] = {0.2f, 0.1f, 0.05f, 1.0f};
    GLfloat corSolEspecular[] = {1.0f, 0.6f, 0.3f, 1.0f};

    // W=0.0 -> luz direcional
    GLfloat posSol[] = {3.0f, 3.0f, 3.0f, 0.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, corSolAmbiente); //define propriedade da cor do sol
    glLightfv(GL_LIGHT0, GL_DIFFUSE, corSolDifusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, corSolEspecular);
    glLightfv(GL_LIGHT0, GL_POSITION, posSol);


    // --- LUZ PONTUAL - POSTE (GL_LIGHT1) ---
    // simula a lampada no topo do poste


    // cor do poste: branco quente (difusa) e posicao pontual
    GLfloat corPosteDifusa[] = {1.2f, 1.05f, 0.85f, 1.0f}; // levemente mais forte para iluminar o chao
    GLfloat corPosteAmbiente[] = {0.02f, 0.02f, 0.02f, 1.0f};
    GLfloat corPosteEspecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    // W=1.0 -> luz pontual
    GLfloat posPoste[] = {-3.0f, 3.1f, 0.0f, 1.0f};

    glLightfv(GL_LIGHT1, GL_AMBIENT, corPosteAmbiente); //define a propriedade da luz do poste 
    glLightfv(GL_LIGHT1, GL_DIFFUSE, corPosteDifusa);
    glLightfv(GL_LIGHT1, GL_SPECULAR, corPosteEspecular);
    glLightfv(GL_LIGHT1, GL_POSITION, posPoste);

    // Atenuacao da luz pontual ajustada para melhor pool de luz no chao
    // diminuimos o termo quadratico e linear para ampliar o alcance perceptivel
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.8f); //diminui intensidade base
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.03f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.005f); // faz a luz sumir de maneira suave
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
    glutPostRedisplay(); 
}

void specialKeys(int key, int x, int y) {
    float moveSpeed = 0.5f; // define a velocidade de movimento
    
    // converte angulos para radianos
    float yawRad = angleYaw * M_PI / 180.0f;
    float pitchRad = anglePitch * M_PI / 180.0f;
    
    // Pitch e ignorado pois se o user olhasse para o chao e apertasse para frente, ele iria para baixo
    // Direção frontal 
    float dirX = sin(yawRad); //cria seta invisivel que aponta para frente da camera
    float dirZ = -cos(yawRad);
    
    // Direção para a direita (perpendicular à frente)
    float rightX = cos(yawRad); // cria seta invisivel que aponta para a direita da camera. permitindo movimento lateral
    float rightZ = sin(yawRad);
    
    
    switch (key) {
        case GLUT_KEY_UP:
            // Avança na direção para a qual está olhando
            cameraX += dirX * moveSpeed; //Soma o vetor frente
            cameraZ += dirZ * moveSpeed;
            break;
        case GLUT_KEY_DOWN:
            // Recua na direção oposta
            cameraX -= dirX * moveSpeed; //Subtrai o vetor frente
            cameraZ -= dirZ * moveSpeed;
            break;
        case GLUT_KEY_LEFT:
            // Move para a esquerda
            cameraX -= rightX * moveSpeed; //subtrai o vetor direita
            cameraZ -= rightZ * moveSpeed;
            break;
        case GLUT_KEY_RIGHT:
            // Move para a direita
            cameraX += rightX * moveSpeed; //soma o vetor direita
            cameraZ += rightZ * moveSpeed;
            break;
    }
    
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
    glutSpecialFunc(specialKeys);      // chama tratamento de setas do teclado
    glutPassiveMotionFunc(mouseMotion); // Para o movimento passivo do mouse
    
    // Posiciona o mouse no centro da tela inicialmente
    glutWarpPointer(windowWidth / 2, windowHeight / 2);
    oldMouseX = windowWidth / 2;
    oldMouseY = windowHeight / 2;
    
    glutMainLoop();

    return 0;
}
