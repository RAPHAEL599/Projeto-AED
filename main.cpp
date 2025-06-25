#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LARGURA_TELA    800
#define ALTURA_TELA     600
#define MAX_PLATAFORMAS 20
#define MAX_PERIGOS     10
#define MAX_PORTAS      2
#define MAX_FASES       10

typedef enum {
    JOGANDO,
    FIM_DE_JOGO,
    VITORIA
} EstadoJogo;

typedef enum {
    FOGO,
    AGUA,
    TERRA
} TipoPerigo;

typedef enum {
    JOGADOR_FOGO,
    JOGADOR_AGUA
} TipoJogador;

typedef struct Jogador {
    TipoJogador tipo;
    Vector2     posicao;
    Vector2     velocidade;
    Color       cor;
    bool        podePular;
} Jogador;

typedef struct Plataforma {
    Rectangle retangulo;
} Plataforma;

typedef struct Perigo {
    Rectangle   retangulo;
    TipoPerigo  tipo;
    Color       cor;
} Perigo;

typedef struct Porta {
    Rectangle    retangulo;
    TipoJogador  tipoJogador;
    Color        cor;
} Porta;

typedef struct Fase {
    Plataforma plataformas[MAX_PLATAFORMAS];
    Perigo     perigos[MAX_PERIGOS];
    Porta      portas[MAX_PORTAS];
    int        numPlataformas;
    int        numPerigos;
    int        numPortas;
    Vector2    posInicialFogo;
    Vector2    posInicialAgua;
} Fase;

// Protótipos
void AtualizarJogador(Jogador *j, Plataforma plat[], int nPlat, float gravidade);
void CarregarFase    (Fase fase, Jogador *fogo, Jogador *agua,
                      Plataforma plat[], int *nPlat,
                      Perigo perigos[], int *nPerigos,
                      Porta portas[], int *nPortas);
void ResolverColisaoJogadores(Jogador *fogo, Jogador *agua);

int main(void) {
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Menino Fogo e Menina Água");

    // --- Definição das fases ---
    Fase fases[MAX_FASES] = {
        // Fase 1
        {
            .plataformas = {
                {{   0, 550, LARGURA_TELA,  50 }},  // chão
                {{   0, 400, LARGURA_TELA-100, 20 }},  // plataforma média
                {{ 100, 250, LARGURA_TELA-100, 20 }},  // plataforma alta
                {{ 200, 320, 100, 20 }},               // estreita 1
                {{ 600, 300, 100, 20 }}                // estreita 2
            },
            .perigos = {
                {{ 300, 530, 150, 20 }, AGUA, SKYBLUE},
                {{ 300, 380, 150, 20 }, FOGO, RED},
                {{ 220, 300,  80, 20 }, AGUA, SKYBLUE},
                {{ 420, 450, 100, 20 }, FOGO, RED},
                {{ 350, 490,  80, 20 }, TERRA, GREEN}
            },
            .portas = {
                {{ LARGURA_TELA - 120, 210, 40, 40 }, JOGADOR_FOGO, (Color){255,100,100,255}},
                {{ LARGURA_TELA -  70, 210, 40, 40 }, JOGADOR_AGUA, (Color){100,100,255,255}}
            },
            .numPlataformas = 5,
            .numPerigos     = 5,
            .numPortas      = 2,
            .posInicialFogo = {  60, 540 },
            .posInicialAgua = { 100, 540 }
        },
        // Fase 2
        {
            .plataformas = {
                {{   0, 550, LARGURA_TELA,  50 }},  // chão
                {{   0, 400, LARGURA_TELA-100, 20 }},  // plataforma média
                {{ 100, 250, LARGURA_TELA-100, 20 }},  // plataforma alta
                {{ 200, 320, 100, 20 }},               // estreita 1
                {{ 600, 300, 100, 20 }}                // estreita 2
            },
            .perigos = {
                {{ 300, 530, 150, 20 }, AGUA, SKYBLUE},
                {{ 300, 380, 150, 20 }, FOGO, RED},
                {{ 220, 300,  80, 20 }, AGUA, SKYBLUE},
                {{ 420, 450, 100, 20 }, FOGO, RED},
                {{ 350, 490,  80, 20 }, TERRA, GREEN}
            },
            .portas = {
                {{ LARGURA_TELA - 120, 210, 40, 40 }, JOGADOR_FOGO, (Color){255,100,100,255}},
                {{ LARGURA_TELA -  70, 210, 40, 40 }, JOGADOR_AGUA, (Color){100,100,255,255}}
            },
            .numPlataformas = 5,
            .numPerigos     = 5,
            .numPortas      = 2,
            .posInicialFogo = {  60, 540 },
            .posInicialAgua = { 100, 540 }
        }
    };
    const int numFasesDefinidas = 2;
    int faseAtualIndex = 0;
    EstadoJogo estadoJogo = JOGANDO;

    Jogador meninoFogo = { JOGADOR_FOGO, {0,0}, {0,0}, MAROON, false };
    Jogador meninaAgua = { JOGADOR_AGUA, {0,0}, {0,0}, BLUE,   false };

    Plataforma plataformasAtuais[MAX_PLATAFORMAS];
    Perigo     perigosAtuais    [MAX_PERIGOS];
    Porta      portasAtuais     [MAX_PORTAS];
    int        numPlataformasAtuais = 0;
    int        numPerigosAtuais     = 0;
    int        numPortasAtuais      = 0;

    // --- Variáveis do diamante, do tempo e das estrelas ---
    Rectangle diamante;
    bool      diamanteColetado   = false;
    int       diamantesColetados = 0;
    double    tempoInicio        = 0.0;
    double    tempoFim           = 0.0;
    bool      progressoCalculado = false;
    int       estrelasObtidas    = 0;   // ***

    // Carrega fase inicial
    CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua,
                 plataformasAtuais, &numPlataformasAtuais,
                 perigosAtuais,     &numPerigosAtuais,
                 portasAtuais,      &numPortasAtuais);

    // Posiciona diamante na plataforma estreita 2 (fase 1) e inicia timer
    if (faseAtualIndex == 0) {
        diamante = (Rectangle){ 600 + 50 - 8, 300 - 16, 16, 16 };
        diamanteColetado   = false;
        diamantesColetados = 0;
        estrelasObtidas    = 0;         // ***
    }
    tempoInicio = GetTime();

    const float gravidade           = 0.10f;
    const float velocidadeMovimento = 4.0f;
    const float forcaPulo           = -5.8f;

    SetTargetFPS(60);

    // --- Loop principal ---
    while (!WindowShouldClose()) {
        // --- Atualização ---
        switch (estadoJogo) {
            case JOGANDO: {
                // Controles
                if (IsKeyDown(KEY_A))    meninoFogo.posicao.x -= velocidadeMovimento;
                if (IsKeyDown(KEY_D))    meninoFogo.posicao.x += velocidadeMovimento;
                if (IsKeyPressed(KEY_W) && meninoFogo.podePular) {
                    meninoFogo.velocidade.y = forcaPulo;
                    meninoFogo.podePular    = false;
                }
                if (IsKeyDown(KEY_LEFT))  meninaAgua.posicao.x -= velocidadeMovimento;
                if (IsKeyDown(KEY_RIGHT)) meninaAgua.posicao.x += velocidadeMovimento;
                if (IsKeyPressed(KEY_UP) && meninaAgua.podePular) {
                    meninaAgua.velocidade.y = forcaPulo;
                    meninaAgua.podePular    = false;
                }

                if (IsKeyPressed(KEY_F1)) {
                    faseAtualIndex++; // Avanca para a proxima fase
                    if (faseAtualIndex < numFasesDefinidas) {
                        // Carrega a proxima fase
                        CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais,
                                     &numPerigosAtuais, portasAtuais, &numPortasAtuais);
                        estadoJogo = JOGANDO;
                }}

                // Física e colisões
                AtualizarJogador(&meninoFogo, plataformasAtuais, numPlataformasAtuais, gravidade);
                AtualizarJogador(&meninaAgua, plataformasAtuais, numPlataformasAtuais, gravidade);
                ResolverColisaoJogadores(&meninoFogo, &meninaAgua);

                // Coleta de diamante
                if (!diamanteColetado) {
                    Rectangle recF = { meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 };
                    Rectangle recA = { meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 };
                    if (CheckCollisionRecs(recF, diamante) ||
                        CheckCollisionRecs(recA, diamante)) {
                        diamanteColetado   = true;
                        diamantesColetados++;
                    }
                }

                // Colisão com perigos
                for (int i = 0; i < numPerigosAtuais; i++) {
                    Rectangle recF = { meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 };
                    Rectangle recA = { meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 };
                    if (perigosAtuais[i].tipo == AGUA && CheckCollisionRecs(recF, perigosAtuais[i].retangulo))
                        estadoJogo = FIM_DE_JOGO;
                    if (perigosAtuais[i].tipo == FOGO && CheckCollisionRecs(recA, perigosAtuais[i].retangulo))
                        estadoJogo = FIM_DE_JOGO;
                    if (perigosAtuais[i].tipo == TERRA &&
                       (CheckCollisionRecs(recF, perigosAtuais[i].retangulo) ||
                        CheckCollisionRecs(recA, perigosAtuais[i].retangulo)))
                        estadoJogo = FIM_DE_JOGO;
                }

                // Vitória se ambos estiverem nas portas
                {
                    bool fogoNaPorta = CheckCollisionRecs(
                        (Rectangle){meninoFogo.posicao.x-10, meninoFogo.posicao.y-20,20,20},
                        portasAtuais[0].retangulo
                    );
                    bool aguaNaPorta = CheckCollisionRecs(
                        (Rectangle){meninaAgua.posicao.x-10, meninaAgua.posicao.y-20,20,20},
                        portasAtuais[1].retangulo
                    );
                    if (fogoNaPorta && aguaNaPorta) estadoJogo = VITORIA;
                }
            } break;

            case FIM_DE_JOGO: {
                if (IsKeyPressed(KEY_ENTER)) {
                    // Reinicia mesma fase
                    CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua,
                                 plataformasAtuais, &numPlataformasAtuais,
                                 perigosAtuais,     &numPerigosAtuais,
                                 portasAtuais,      &numPortasAtuais);
                    if (faseAtualIndex == 0) {
                        diamanteColetado   = false;
                        diamantesColetados = 0;
                    }
                    tempoInicio        = GetTime();
                    progressoCalculado = false;
                    estrelasObtidas    = 0;       // ***
                    estadoJogo         = JOGANDO;
                }
            } break;

            case VITORIA: {
                if (!progressoCalculado) {
                    tempoFim = GetTime();

                    // Cálculo de estrelas apenas na Fase 1
                    if (faseAtualIndex == 0) {
                        double duracao = tempoFim - tempoInicio;
                        if (!diamanteColetado) {
                            estrelasObtidas = 0;
                        } else {
                            if (duracao < 20.0)      estrelasObtidas = 3;
                            else if (duracao < 40.0) estrelasObtidas = 2;
                            else                     estrelasObtidas = 1;
                        }
                    } else {
                        estrelasObtidas = 0;
                    }

                    progressoCalculado = true;
                }
                if (IsKeyPressed(KEY_ENTER)) {
                    faseAtualIndex++;
                    if (faseAtualIndex < numFasesDefinidas) {
                        CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua,
                                     plataformasAtuais, &numPlataformasAtuais,
                                     perigosAtuais,     &numPerigosAtuais,
                                     portasAtuais,      &numPortasAtuais);
                        if (faseAtualIndex == 0) {
                            diamante = (Rectangle){ 600 + 50 - 8, 300 - 16, 16, 16 };
                            diamanteColetado   = false;
                            diamantesColetados = 0;
                        }
                        tempoInicio        = GetTime();
                        progressoCalculado = false;
                        estrelasObtidas    = 0;   // ***
                        estadoJogo         = JOGANDO;
                    } else {
                        CloseWindow();
                    }
                }
            } break;
        }

        // --- Desenho ---
        BeginDrawing();
            ClearBackground((Color){240,240,240,255});

            // Plataformas, perigos e portas
            for (int i = 0; i < numPlataformasAtuais; i++)
                DrawRectangleRec(plataformasAtuais[i].retangulo, DARKGRAY);
            for (int i = 0; i < numPerigosAtuais; i++)
                DrawRectangleRec(perigosAtuais[i].retangulo, perigosAtuais[i].cor);
            for (int i = 0; i < numPortasAtuais; i++)
                DrawRectangleRec(portasAtuais[i].retangulo, portasAtuais[i].cor);

            // Jogadores
            DrawRectangle(meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20, meninoFogo.cor);
            DrawRectangle(meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20, meninaAgua.cor);

            // Diamante
            if (estadoJogo == JOGANDO && !diamanteColetado) {
                DrawRectangleRec(diamante, GOLD);
            }

            // HUD
            DrawText(TextFormat("Fase %d", faseAtualIndex + 1),
                     LARGURA_TELA - 100, 10, 20, LIGHTGRAY);

            if (estadoJogo == JOGANDO) {
                DrawText("Fogo: WASD | Agua: Setas", 10, 10, 20, DARKGRAY);
            }
            else if (estadoJogo == FIM_DE_JOGO) {
                DrawText("FIM DE JOGO",
                         LARGURA_TELA/2 - MeasureText("FIM DE JOGO",40)/2,
                         ALTURA_TELA/2 - 40, 40, GRAY);
                DrawText("Pressione ENTER para reiniciar a fase",
                         LARGURA_TELA/2 - MeasureText("Pressione ENTER para reiniciar a fase",20)/2,
                         ALTURA_TELA/2 + 10, 20, GRAY);
            }
            else if (estadoJogo == VITORIA) {
                // 1) Fundo semi-transparente
                int boxW = 400, boxH = 220;
                int boxX = LARGURA_TELA/2 - boxW/2;
                int boxY = ALTURA_TELA/2 - 80;
                DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.5f));

                // 2) Título com sombra
                const char *titulo = "VITORIA!";
                int fsTitle = 40;
                int wTitle  = MeasureText(titulo, fsTitle);
                // Sombra preta
                DrawText(titulo, LARGURA_TELA/2 - wTitle/2 + 2, ALTURA_TELA/2 - 50 + 2, fsTitle, BLACK);
                // Texto dourado
                DrawText(titulo, LARGURA_TELA/2 - wTitle/2,     ALTURA_TELA/2 - 50,     fsTitle, GOLD);

                // 3) Instrução para próxima fase
                const char *instr = (faseAtualIndex + 1 < numFasesDefinidas)
                    ? "Pressione ENTER para a proxima fase"
                    : "Parabens! Você completou o jogo!";
                int fsInstr = 20;
                int wInstr  = MeasureText(instr, fsInstr);
                DrawText(instr, LARGURA_TELA/2 - wInstr/2, ALTURA_TELA/2 - 10, fsInstr, GOLD);

                // 4) Estatísticas com fonte maior e em branco
                int fsStat = 28;
                char buf[64];
                int y0 = ALTURA_TELA/2 + 30;

                // Tempo
                sprintf(buf, "Tempo: %.2f s", tempoFim - tempoInicio);
                int wx = MeasureText(buf, fsStat);
                DrawText(buf, LARGURA_TELA/2 - wx/2, y0 +   0, fsStat, WHITE);

                // Diamantes
                sprintf(buf, "Diamantes: %d", diamantesColetados);
                wx = MeasureText(buf, fsStat);
                DrawText(buf, LARGURA_TELA/2 - wx/2, y0 +  40, fsStat, WHITE);

                // Estrelas
                sprintf(buf, "Estrelas: %d", estrelasObtidas);
                wx = MeasureText(buf, fsStat);
                DrawText(buf, LARGURA_TELA/2 - wx/2, y0 +  80, fsStat, WHITE);
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void CarregarFase(Fase fase, Jogador *fogo, Jogador *agua,
                  Plataforma plat[], int *nPlat,
                  Perigo     perigos[], int *nPerigos,
                  Porta      portas[],  int *nPortas) {
    fogo->posicao       = fase.posInicialFogo;
    agua->posicao       = fase.posInicialAgua;
    fogo->velocidade    = (Vector2){0};
    agua->velocidade    = (Vector2){0};
    *nPlat     = fase.numPlataformas;
    *nPerigos  = fase.numPerigos;
    *nPortas   = fase.numPortas;
    for (int i = 0; i < *nPlat;     i++) plat[i]    = fase.plataformas[i];
    for (int i = 0; i < *nPerigos;  i++) perigos[i] = fase.perigos[i];
    for (int i = 0; i < *nPortas;   i++) portas[i]  = fase.portas[i];
}

void ResolverColisaoJogadores(Jogador *fogo, Jogador *agua) {
    Rectangle recF = { fogo->posicao.x - 10, fogo->posicao.y - 20, 20, 20 };
    Rectangle recA = { agua->posicao.x - 10, agua->posicao.y - 20, 20, 20 };
    if (CheckCollisionRecs(recF, recA)) {
        Rectangle overlap = GetCollisionRec(recF, recA);
        if (overlap.width < overlap.height) {
            float shift = overlap.width/2.0f;
            if (recF.x < recA.x) {
                fogo->posicao.x -= shift;
                agua->posicao.x -= shift * -1;
            } else {
                fogo->posicao.x += shift;
                agua->posicao.x -= shift;
            }
        } else {
            if (fogo->velocidade.y > 0 && recF.y < recA.y) {
                fogo->posicao.y    = recA.y;
                fogo->velocidade.y = 0;
                fogo->podePular    = true;
            } else if (agua->velocidade.y > 0 && recA.y < recF.y) {
                agua->posicao.y    = recF.y;
                agua->velocidade.y = 0;
                agua->podePular    = true;
            }
        }
    }
}

void AtualizarJogador(Jogador *j, Plataforma plat[], int nPlat, float gravidade) {
    j->posicao.x    += j->velocidade.x;
    j->velocidade.y += gravidade;
    j->posicao.y    += j->velocidade.y;

    float h = 20.0f, w = 20.0f;
    Rectangle rec = { j->posicao.x - w/2, j->posicao.y - h, w, h };
    j->podePular = false;

    for (int i = 0; i < nPlat; i++) {
        Rectangle p = plat[i].retangulo;
        if (CheckCollisionRecs(rec, p)) {
            // Atingiu de cima
            if (j->velocidade.y > 0 && (rec.y + h - j->velocidade.y) <= p.y) {
                j->posicao.y    = p.y;
                j->velocidade.y = 0;
                j->podePular    = true;
            }
            // Atingiu de baixo
            else if (j->velocidade.y < 0 && rec.y > (p.y + p.height - 5)) {
                j->posicao.y    = p.y + p.height + h;
                j->velocidade.y = 0;
            }
        }
    }
}
