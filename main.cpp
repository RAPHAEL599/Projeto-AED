#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define MAX_PLATAFORMAS 20
#define MAX_PERIGOS 10
#define MAX_PORTAS 2
#define MAX_FASES 3
#define MAX_BOTOES 5
#define MAX_PLATAFORMAS_MOVEIS 5

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
    Vector2 posicao;
    Vector2 velocidade;
    Color cor;
    bool podePular;
} Jogador;

typedef struct Plataforma {
    Rectangle retangulo;
} Plataforma;

typedef struct Perigo {
    Rectangle retangulo;
    TipoPerigo tipo;
    Color cor;
} Perigo;

typedef struct Porta {
    Rectangle retangulo;
    TipoJogador tipoJogador;
    Color cor;
} Porta;

// --- NOVAS ESTRUTURAS PARA INTERATIVIDADE ---
typedef struct Botao {
    Rectangle retangulo;
    int idAlvo;             // ID da plataforma móvel que este botão controla
    bool pressionado;
    Color cor;
} Botao;

typedef struct PlataformaMovel {
    Rectangle retangulo;
    Vector2 posInicial;
    Vector2 posFinal;
    bool ativa;             // Se está se movendo para o destino (posFinal)
    float velocidade;
} PlataformaMovel;


typedef struct Fase {
    Plataforma plataformas[MAX_PLATAFORMAS];
    Perigo perigos[MAX_PERIGOS];
    Porta portas[MAX_PORTAS];
    // --- NOVOS ELEMENTOS DA FASE ---
    Botao botoes[MAX_BOTOES];
    PlataformaMovel plataformasMoveis[MAX_PLATAFORMAS_MOVEIS];

    int numPlataformas;
    int numPerigos;
    int numPortas;
    // --- NOVAS CONTAGENS ---
    int numBotoes;
    int numPlataformasMoveis;

    Vector2 posInicialFogo;
    Vector2 posInicialAgua;
    bool temDiamante;
    Rectangle diamante;
} Fase;

// --- PROTÓTIPOS ATUALIZADOS ---
void CarregarFase(Fase fase, Jogador *fogo, Jogador *agua,
                  Plataforma plat[], int *nPlat,
                  Perigo perigos[], int *nPerigos,
                  Porta portas[], int *nPortas,
                  Botao botoes[], int *nBotoes,
                  PlataformaMovel platMoveis[], int *nPlatMoveis,
                  bool *temDiamante, Rectangle *diamanteRect);

void ResolverColisaoJogadores(Jogador *fogo, Jogador *agua);
// --- ATUALIZADO PARA INCLUIR PLATAFORMAS MÓVEIS ---
void AtualizarJogador(Jogador *j, Plataforma plat[], int nPlat, PlataformaMovel platMoveis[], int nPlatMoveis, float gravidade);


int main(void) {
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Menino Fogo e Menina Água - Pontuação Padronizada");

    Fase fases[MAX_FASES] = {
        // Fase 1
        {
            .plataformas = {
                {{ 0, 550, LARGURA_TELA, 50 }},
                {{ 0, 400, LARGURA_TELA - 100, 20 }},
                {{ 100, 250, LARGURA_TELA - 100, 20 }},
                {{ 200, 320, 100, 20 }},
                {{ 600, 300, 100, 20 }}
            },
            .perigos = {
                {{ 300, 530, 150, 20 }, AGUA, SKYBLUE},
                {{ 300, 380, 150, 20 }, FOGO, RED},
                {{ 220, 300, 80, 20 }, AGUA, SKYBLUE},
                {{ 420, 450, 100, 20 }, FOGO, RED},
                {{ 350, 490, 80, 20 }, TERRA, GREEN}
            },
            .portas = {
                {{ LARGURA_TELA - 120, 210, 40, 40 }, JOGADOR_FOGO, (Color){255,100,100,255}},
                {{ LARGURA_TELA - 70, 210, 40, 40 }, JOGADOR_AGUA, (Color){100,100,255,255}}
            },
            .numPlataformas = 5,
            .numPerigos = 5,
            .numPortas = 2,
            .numBotoes = 0,
            .numPlataformasMoveis = 0,
            .posInicialFogo = { 60, 540 },
            .posInicialAgua = { 100, 540 },
            .temDiamante = true,
            .diamante = { 600 + 50 - 8, 300 - 16, 16, 16 }
        },
        // Fase 2
        {
            .plataformas = {
                { { 0, 580, LARGURA_TELA, 20 } },
                { { 0, 450, 250, 20 } },
                { { 350, 450, 450, 20 } },
                { { 450, 300, 150, 20 } },
                { { 600, 200, 200, 20 } }
            },
            .perigos = {
                { { 280, 560, 150, 20 }, AGUA, SKYBLUE },
                { { 450, 280, 150, 20 }, TERRA, GREEN }
            },
            .portas = {
                { { LARGURA_TELA - 140, 160, 40, 40 }, JOGADOR_FOGO, (Color){255,100,100,255} },
                { { LARGURA_TELA - 90, 160, 40, 40 }, JOGADOR_AGUA, (Color){100,100,255,255} }
            },
            .botoes = {
                { { 100, 430, 50, 10 }, .idAlvo = 0, .pressionado = false, .cor = DARKBLUE },
                { { 750, 430, 50, 10 }, .idAlvo = 1, .pressionado = false, .cor = ORANGE },
                { { 695, 190, 50, 10 }, .idAlvo = 1, .pressionado = false, .cor = PURPLE }
            },
            .plataformasMoveis = {
                { .retangulo = { 300, 370, 20, 100 }, .posInicial = {300, 370}, .posFinal = {300, 270}, .ativa = false, .velocidade = 1.0f},
                { .retangulo = { 500, 430, 50, 20 }, .posInicial = {500, 430}, .posFinal = {500, 220}, .ativa = false, .velocidade = 1.5f}
            },
            .numPlataformas = 5,
            .numPerigos = 3,
            .numPortas = 2,
            .numBotoes = 3,
            .numPlataformasMoveis = 2,
            .posInicialFogo = { 60, 570 },
            .posInicialAgua = { 100, 570 },
            .temDiamante = true,
            .diamante = { 758, 414, 16, 16 }
        }
    };
    const int numFasesDefinidas = 2;
    int faseAtualIndex = 0;
    EstadoJogo estadoJogo = JOGANDO;

    Jogador meninoFogo = { JOGADOR_FOGO, {0,0}, {0,0}, MAROON, false };
    Jogador meninaAgua = { JOGADOR_AGUA, {0,0}, {0,0}, BLUE, false };

    Plataforma plataformasAtuais[MAX_PLATAFORMAS];
    Perigo perigosAtuais[MAX_PERIGOS];
    Porta portasAtuais[MAX_PORTAS];
    Botao botoesAtuais[MAX_BOTOES];
    PlataformaMovel plataformasMoveisAtuais[MAX_PLATAFORMAS_MOVEIS];

    int numPlataformasAtuais = 0;
    int numPerigosAtuais = 0;
    int numPortasAtuais = 0;
    int numBotoesAtuais = 0;
    int numPlataformasMoveisAtuais = 0;

    Rectangle diamante;
    bool temDiamanteAtual = false;
    bool diamanteColetado = false;
    int diamantesColetados = 0;
    double tempoInicio = 0.0;
    double tempoFim = 0.0;
    bool progressoCalculado = false;
    int estrelasObtidas = 0;

    CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua,
                 plataformasAtuais, &numPlataformasAtuais,
                 perigosAtuais, &numPerigosAtuais,
                 portasAtuais, &numPortasAtuais,
                 botoesAtuais, &numBotoesAtuais,
                 plataformasMoveisAtuais, &numPlataformasMoveisAtuais,
                 &temDiamanteAtual, &diamante);

    diamanteColetado = false;
    diamantesColetados = 0;
    estrelasObtidas = 0;
    tempoInicio = GetTime();

    const float gravidade = 0.10f;
    const float velocidadeMovimento = 4.0f;
    const float forcaPulo = -5.8f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        switch (estadoJogo) {
            case JOGANDO: {
                 if (IsKeyDown(KEY_A)) meninoFogo.posicao.x -= velocidadeMovimento;
                 if (IsKeyDown(KEY_D)) meninoFogo.posicao.x += velocidadeMovimento;
                 if (IsKeyPressed(KEY_W) && meninoFogo.podePular) {
                     meninoFogo.velocidade.y = forcaPulo;
                     meninoFogo.podePular = false;
                 }
                 if (IsKeyDown(KEY_LEFT)) meninaAgua.posicao.x -= velocidadeMovimento;
                 if (IsKeyDown(KEY_RIGHT)) meninaAgua.posicao.x += velocidadeMovimento;
                 if (IsKeyPressed(KEY_UP) && meninaAgua.podePular) {
                     meninaAgua.velocidade.y = forcaPulo;
                     meninaAgua.podePular = false;
                 }

                if (IsKeyPressed(KEY_F1)) {
                    faseAtualIndex = (faseAtualIndex + 1) % numFasesDefinidas;
                    CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais,
                                 &numPerigosAtuais, portasAtuais, &numPortasAtuais, botoesAtuais, &numBotoesAtuais,
                                 plataformasMoveisAtuais, &numPlataformasMoveisAtuais, &temDiamanteAtual, &diamante);
                    diamanteColetado = false;
                    diamantesColetados = 0;
                    tempoInicio = GetTime();
                    progressoCalculado = false;
                    estrelasObtidas = 0;
                    estadoJogo = JOGANDO;
                }

                for (int i = 0; i < numBotoesAtuais; i++) {
                    botoesAtuais[i].pressionado = false;
                }

                Rectangle recF = { meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 };
                Rectangle recA = { meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 };

                for (int i = 0; i < numBotoesAtuais; i++) {
                    if (CheckCollisionRecs(recF, botoesAtuais[i].retangulo) || CheckCollisionRecs(recA, botoesAtuais[i].retangulo)) {
                        botoesAtuais[i].pressionado = true;
                    }
                }

                for (int i = 0; i < numPlataformasMoveisAtuais; i++) {
                    plataformasMoveisAtuais[i].ativa = false;
                }
                for (int i = 0; i < numBotoesAtuais; i++) {
                    if (botoesAtuais[i].pressionado) {
                        int idAlvo = botoesAtuais[i].idAlvo;
                        if (idAlvo >= 0 && idAlvo < numPlataformasMoveisAtuais) {
                            plataformasMoveisAtuais[idAlvo].ativa = true;
                        }
                    }
                }


                for (int i = 0; i < numPlataformasMoveisAtuais; i++) {
                    PlataformaMovel *p = &plataformasMoveisAtuais[i];
                    if (p->ativa) {
                        if (p->retangulo.x < p->posFinal.x) p->retangulo.x = fmin(p->retangulo.x + p->velocidade, p->posFinal.x);
                        if (p->retangulo.x > p->posFinal.x) p->retangulo.x = fmax(p->retangulo.x - p->velocidade, p->posFinal.x);
                        if (p->retangulo.y < p->posFinal.y) p->retangulo.y = fmin(p->retangulo.y + p->velocidade, p->posFinal.y);
                        if (p->retangulo.y > p->posFinal.y) p->retangulo.y = fmax(p->retangulo.y - p->velocidade, p->posFinal.y);
                    } else {
                        if (p->retangulo.x < p->posInicial.x) p->retangulo.x = fmin(p->retangulo.x + p->velocidade, p->posInicial.x);
                        if (p->retangulo.x > p->posInicial.x) p->retangulo.x = fmax(p->retangulo.x - p->velocidade, p->posInicial.x);
                        if (p->retangulo.y < p->posInicial.y) p->retangulo.y = fmin(p->retangulo.y + p->velocidade, p->posInicial.y);
                        if (p->retangulo.y > p->posInicial.y) p->retangulo.y = fmax(p->retangulo.y - p->velocidade, p->posInicial.y);
                    }
                }

                AtualizarJogador(&meninoFogo, plataformasAtuais, numPlataformasAtuais, plataformasMoveisAtuais, numPlataformasMoveisAtuais, gravidade);
                AtualizarJogador(&meninaAgua, plataformasAtuais, numPlataformasAtuais, plataformasMoveisAtuais, numPlataformasMoveisAtuais, gravidade);
                ResolverColisaoJogadores(&meninoFogo, &meninaAgua);

                if (temDiamanteAtual && !diamanteColetado) {
                    if (CheckCollisionRecs(recF, diamante) || CheckCollisionRecs(recA, diamante)) {
                        diamanteColetado = true;
                        diamantesColetados++;
                    }
                }

                for (int i = 0; i < numPerigosAtuais; i++) {
                    if (perigosAtuais[i].tipo == AGUA && CheckCollisionRecs(recF, perigosAtuais[i].retangulo))
                        estadoJogo = FIM_DE_JOGO;
                    if (perigosAtuais[i].tipo == FOGO && CheckCollisionRecs(recA, perigosAtuais[i].retangulo))
                        estadoJogo = FIM_DE_JOGO;
                    if (perigosAtuais[i].tipo == TERRA && (CheckCollisionRecs(recF, perigosAtuais[i].retangulo) || CheckCollisionRecs(recA, perigosAtuais[i].retangulo)))
                        estadoJogo = FIM_DE_JOGO;
                }

                bool fogoNaPorta = CheckCollisionRecs((Rectangle){meninoFogo.posicao.x-10, meninoFogo.posicao.y-20,20,20}, portasAtuais[0].retangulo);
                bool aguaNaPorta = CheckCollisionRecs((Rectangle){meninaAgua.posicao.x-10, meninaAgua.posicao.y-20,20,20}, portasAtuais[1].retangulo);
                if (fogoNaPorta && aguaNaPorta) estadoJogo = VITORIA;

            } break;

            case FIM_DE_JOGO: {
                 if (IsKeyPressed(KEY_ENTER)) {
                     CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais,
                                  &numPerigosAtuais, portasAtuais, &numPortasAtuais, botoesAtuais, &numBotoesAtuais,
                                  plataformasMoveisAtuais, &numPlataformasMoveisAtuais, &temDiamanteAtual, &diamante);
                     diamanteColetado = false;
                     diamantesColetados = 0;
                     tempoInicio = GetTime();
                     progressoCalculado = false;
                     estrelasObtidas = 0;
                     estadoJogo = JOGANDO;
                 }
            } break;

            case VITORIA: {
                if (!progressoCalculado) {
                    tempoFim = GetTime();
                    double duracao = tempoFim - tempoInicio;

                    // <-- ALTERAÇÃO: Lógica de pontuação ajustada para corresponder ao pedido
                    if (temDiamanteAtual && !diamanteColetado) {
                        estrelasObtidas = 0;
                    } else {
                        if (duracao < 20.0)      estrelasObtidas = 3;
                        else if (duracao < 40.0) estrelasObtidas = 2;
                        else                     estrelasObtidas = 1;
                    }
                    progressoCalculado = true;
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    faseAtualIndex++;
                    if (faseAtualIndex < numFasesDefinidas) {
                        CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais,
                                     &numPerigosAtuais, portasAtuais, &numPortasAtuais, botoesAtuais, &numBotoesAtuais,
                                     plataformasMoveisAtuais, &numPlataformasMoveisAtuais, &temDiamanteAtual, &diamante);
                        diamanteColetado = false;
                        diamantesColetados = 0;
                        tempoInicio = GetTime();
                        progressoCalculado = false;
                        estrelasObtidas = 0;
                        estadoJogo = JOGANDO;
                    } else {
                        CloseWindow();
                    }
                }
            } break;
        }

        BeginDrawing();
            ClearBackground((Color){240,240,240,255});

            for (int i = 0; i < numPlataformasAtuais; i++)
                DrawRectangleRec(plataformasAtuais[i].retangulo, DARKGRAY);
            for (int i = 0; i < numPlataformasMoveisAtuais; i++)
                DrawRectangleRec(plataformasMoveisAtuais[i].retangulo, (Color){100, 100, 100, 255});
            for (int i = 0; i < numBotoesAtuais; i++)
                DrawRectangleRec(botoesAtuais[i].retangulo, botoesAtuais[i].pressionado ? LIME : botoesAtuais[i].cor);

            for (int i = 0; i < numPerigosAtuais; i++)
                DrawRectangleRec(perigosAtuais[i].retangulo, perigosAtuais[i].cor);
            for (int i = 0; i < numPortasAtuais; i++)
                DrawRectangleRec(portasAtuais[i].retangulo, portasAtuais[i].cor);

            DrawRectangle(meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20, meninoFogo.cor);
            DrawRectangle(meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20, meninaAgua.cor);

            if (estadoJogo == JOGANDO && temDiamanteAtual && !diamanteColetado) {
                DrawRectangleRec(diamante, GOLD);
            }

            DrawText(TextFormat("Fase %d", faseAtualIndex + 1), LARGURA_TELA - 100, 10, 20, LIGHTGRAY);
            if (estadoJogo == JOGANDO) {
                DrawText("Fogo: WASD | Agua: Setas | F1: Prox Fase", 10, 10, 20, DARKGRAY);
            } else if (estadoJogo == FIM_DE_JOGO) {
                DrawText("FIM DE JOGO", LARGURA_TELA/2 - MeasureText("FIM DE JOGO",40)/2, ALTURA_TELA/2 - 40, 40, GRAY);
                DrawText("Pressione ENTER para reiniciar a fase", LARGURA_TELA/2 - MeasureText("Pressione ENTER para reiniciar a fase",20)/2, ALTURA_TELA/2 + 10, 20, GRAY);
            } else if (estadoJogo == VITORIA) {
                int boxW = 400, boxH = 220;
                int boxX = LARGURA_TELA/2 - boxW/2;
                int boxY = ALTURA_TELA/2 - 80;
                DrawRectangle(boxX, boxY, boxW, boxH, Fade(BLACK, 0.5f));
                const char *titulo = "VITORIA!";
                int fsTitle = 40;
                int wTitle = MeasureText(titulo, fsTitle);
                DrawText(titulo, LARGURA_TELA/2 - wTitle/2 + 2, ALTURA_TELA/2 - 50 + 2, fsTitle, BLACK);
                DrawText(titulo, LARGURA_TELA/2 - wTitle/2, ALTURA_TELA/2 - 50, fsTitle, GOLD);
                const char *instr = (faseAtualIndex + 1 < numFasesDefinidas) ? "Pressione ENTER para a proxima fase" : "Parabens! Você completou o jogo!";
                int fsInstr = 20;
                int wInstr = MeasureText(instr, fsInstr);
                DrawText(instr, LARGURA_TELA/2 - wInstr/2, ALTURA_TELA/2 - 10, fsInstr, GOLD);
                int fsStat = 28;
                char buf[64];
                int y0 = ALTURA_TELA/2 + 30;
                sprintf(buf, "Tempo: %.2f s", tempoFim - tempoInicio);
                int wx = MeasureText(buf, fsStat);
                DrawText(buf, LARGURA_TELA/2 - wx/2, y0 + 0, fsStat, WHITE);
                sprintf(buf, "Diamantes: %d", diamantesColetados);
                wx = MeasureText(buf, fsStat);
                DrawText(buf, LARGURA_TELA/2 - wx/2, y0 + 40, fsStat, WHITE);
                sprintf(buf, "Estrelas: %d", estrelasObtidas);
                wx = MeasureText(buf, fsStat);
                DrawText(buf, LARGURA_TELA/2 - wx/2, y0 + 80, fsStat, WHITE);
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

// O resto do código permanece inalterado...

void CarregarFase(Fase fase, Jogador *fogo, Jogador *agua,
                  Plataforma plat[], int *nPlat,
                  Perigo perigos[], int *nPerigos,
                  Porta portas[], int *nPortas,
                  Botao botoes[], int *nBotoes,
                  PlataformaMovel platMoveis[], int *nPlatMoveis,
                  bool *temDiamante, Rectangle *diamanteRect) {
    fogo->posicao = fase.posInicialFogo;
    agua->posicao = fase.posInicialAgua;
    fogo->velocidade = (Vector2){0};
    agua->velocidade = (Vector2){0};
    *nPlat = fase.numPlataformas;
    *nPerigos = fase.numPerigos;
    *nPortas = fase.numPortas;
    *nBotoes = fase.numBotoes;
    *nPlatMoveis = fase.numPlataformasMoveis;

    for (int i = 0; i < *nPlat; i++) plat[i] = fase.plataformas[i];
    for (int i = 0; i < *nPerigos; i++) perigos[i] = fase.perigos[i];
    for (int i = 0; i < *nPortas; i++) portas[i] = fase.portas[i];
    for (int i = 0; i < *nBotoes; i++) botoes[i] = fase.botoes[i];
    for (int i = 0; i < *nPlatMoveis; i++) platMoveis[i] = fase.plataformasMoveis[i];

    *temDiamante = fase.temDiamante;
    *diamanteRect = fase.diamante;
}

void ResolverColisaoJogadores(Jogador *fogo, Jogador *agua) {
    Rectangle recF = { fogo->posicao.x - 10, fogo->posicao.y - 20, 20, 20 };
    Rectangle recA = { agua->posicao.x - 10, agua->posicao.y - 20, 20, 20 };
    if (CheckCollisionRecs(recF, recA)) {
        Rectangle overlap = GetCollisionRec(recF, recA);
        if (overlap.width < overlap.height) {
            float shift = overlap.width / 2.0f;
            if (recF.x < recA.x) {
                fogo->posicao.x -= shift;
                agua->posicao.x += shift;
            } else {
                fogo->posicao.x += shift;
                agua->posicao.x -= shift;
            }
        } else {
            if (fogo->velocidade.y > 0 && recF.y < recA.y) {
                fogo->posicao.y = recA.y;
                fogo->velocidade.y = 0;
                fogo->podePular = true;
            } else if (agua->velocidade.y > 0 && recA.y < recF.y) {
                agua->posicao.y = recF.y;
                agua->velocidade.y = 0;
                agua->podePular = true;
            }
        }
    }
}

void AtualizarJogador(Jogador *j, Plataforma plat[], int nPlat, PlataformaMovel platMoveis[], int nPlatMoveis, float gravidade) {
    j->posicao.x += j->velocidade.x;
    j->velocidade.y += gravidade;
    j->posicao.y += j->velocidade.y;

    float h = 20.0f, w = 20.0f;
    Rectangle rec = { j->posicao.x - w / 2, j->posicao.y - h, w, h };
    j->podePular = false;

    // Colisão com plataformas estáticas
    for (int i = 0; i < nPlat; i++) {
        Rectangle p = plat[i].retangulo;
        if (CheckCollisionRecs(rec, p)) {
            if (j->velocidade.y > 0 && (rec.y + h - j->velocidade.y) <= p.y) {
                j->posicao.y = p.y;
                j->velocidade.y = 0;
                j->podePular = true;
            } else if (j->velocidade.y < 0 && rec.y > (p.y + p.height - 5)) {
                j->posicao.y = p.y + p.height + h;
                j->velocidade.y = 0;
            }
        }
    }

    // Colisão com plataformas móveis
    for (int i = 0; i < nPlatMoveis; i++) {
        Rectangle p = platMoveis[i].retangulo;
        if (CheckCollisionRecs(rec, p)) {
            if (j->velocidade.y > 0 && (rec.y + h - j->velocidade.y) <= p.y) {
                j->posicao.y = p.y;
                j->velocidade.y = 0;
                j->podePular = true;
                if (platMoveis[i].ativa) {
                    if (platMoveis[i].posInicial.x != platMoveis[i].posFinal.x) j->posicao.x += platMoveis[i].velocidade * (platMoveis[i].posFinal.x > platMoveis[i].posInicial.x ? 1 : -1);
                }
            } else if (j->velocidade.y < 0 && rec.y > (p.y + p.height - 5)) {
                j->posicao.y = p.y + p.height + h;
                j->velocidade.y = 0;
            }
        }
    }
}
