#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
//#include <stdio.h>

#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define MAX_PLATAFORMAS 20 // Definindo limites para os vetores na struct
#define MAX_PERIGOS 10
#define MAX_PORTAS 2

// --- Estruturas de Dados ---

typedef enum {
    JOGANDO,
    FIM_DE_JOGO,
    VITORIA
} EstadoJogo;

typedef enum {
    FOGO,
    AGUA
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

typedef struct Fase {
    Plataforma plataformas[MAX_PLATAFORMAS];
    Perigo perigos[MAX_PERIGOS];
    Porta portas[MAX_PORTAS];
    int numPlataformas;
    int numPerigos;
    int numPortas;
    Vector2 posInicialFogo;
    Vector2 posInicialAgua;
} Fase;


// --- Fun��es de Ajuda ---
void AtualizarJogador(Jogador *jogador, Plataforma plataformas[], int numPlataformas, float gravidade);
void CarregarFase(Fase fase, Jogador *meninoFogo, Jogador *meninaAgua, Plataforma plataformas[], int *numPlataformas, Perigo perigos[],
                  int *numPerigos, Porta portas[], int *numPortas);


// --- Fun��o Principal ---
int main(void) {
    // Inicializa��o
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Menino Fogo e Menina �gua - M�ltiplas Fases");

    // --- DEFINI��O DE TODAS AS FASES DO JOGO ---
    Fase fases[] = {
        // --- FASE 1 ---
        {
            .plataformas = {
                // 1. Caminho de baixo (Esquerda -> Direita) - Sem altera��es
                {{ 0, 550, LARGURA_TELA, 50 }},

                // 2. Caminho do meio (Direita -> Esquerda)
                // CORRE��O: Come�a na esquerda e deixa o buraco na DIREITA para subir.
                {{ 0, 400, LARGURA_TELA - 100, 20 }},

                // 3. Caminho de cima (Esquerda -> Direita), onde ficam as portas
                // CORRE��O: Come�a depois da borda e deixa o buraco na ESQUERDA para subir.
                {{ 100, 250, LARGURA_TELA - 100, 20 }}
            },
            .perigos = {
                // Perigo no caminho de baixo
                {{ 300, 530, 150, 20 }, AGUA, SKYBLUE},

                // Perigo no caminho do meio
                {{ 300, 380, 150, 20 }, FOGO, RED}
            },
            .portas = {
                {{ LARGURA_TELA - 120, 210, 40, 40 }, JOGADOR_FOGO, (Color){ 255, 100, 100, 255 }},
                {{ LARGURA_TELA - 70, 210, 40, 40 }, JOGADOR_AGUA, (Color){ 100, 100, 255, 255 }}
            },
            .numPlataformas = 3,
            .numPerigos = 2,
            .numPortas = 2,
            .posInicialFogo = { 60, 540 },
            .posInicialAgua = { 100, 540 }
        },
        // --- FASE 2 ---
        {
            .plataformas = {
                // 1. Caminho de cima (Esquerda -> Direita) com buraco na DIREITA para descer.
                {{ 0, 250, LARGURA_TELA - 100, 20 }},

                // 2. Caminho do meio (Direita -> Esquerda) com buraco na ESQUERDA para descer.
                {{ 100, 400, LARGURA_TELA - 100, 20 }},

                // 3. Caminho de baixo (Esquerda -> Direita), o ch�o final.
                {{ 0, 550, LARGURA_TELA, 50 }}
            },
            .perigos = {
                // Perigo no caminho de cima
                {{ 300, 230, 150, 20 }, AGUA, SKYBLUE},
                // Perigo no caminho do meio
                {{ 300, 380, 150, 20 }, FOGO, RED}
            },
            .portas = {
                // Portas no canto inferior direito, no final do percurso
                {{ LARGURA_TELA - 120, 510, 40, 40 }, JOGADOR_FOGO, (Color){ 255, 100, 100, 255 }},
                {{ LARGURA_TELA - 70, 510, 40, 40 }, JOGADOR_AGUA, (Color){ 100, 100, 255, 255 }}
            },
            .numPlataformas = 3,
            .numPerigos = 2,
            .numPortas = 2,
            // Come�am no canto superior esquerdo, na primeira plataforma
            .posInicialFogo = { 60, 240 },
            .posInicialAgua = { 100, 240 }
        },
        // --- FASE 3 ---
        {
            .plataformas = {
                // 1. Caminho de cima (Esquerda -> Direita) com buraco na DIREITA para descer.
                {{ 0, 250, LARGURA_TELA - 100, 20 }},

                // 2. Caminho do meio (Direita -> Esquerda) com buraco na ESQUERDA para descer.
                {{ 100, 400, LARGURA_TELA - 100, 20 }},

                // 3. Caminho de baixo (Esquerda -> Direita), o ch�o final.
                {{ 0, 550, LARGURA_TELA, 50 }}
            },
            .perigos = {
                // Perigo no caminho de cima
                {{ 300, 230, 150, 20 }, AGUA, SKYBLUE},
                // Perigo no caminho do meio
                {{ 300, 380, 150, 20 }, FOGO, RED}
            },
            .portas = {
                // Portas no canto inferior direito, no final do percurso
                {{ LARGURA_TELA - 120, 510, 40, 40 }, JOGADOR_FOGO, (Color){ 255, 100, 100, 255 }},
                {{ LARGURA_TELA - 70, 510, 40, 40 }, JOGADOR_AGUA, (Color){ 100, 100, 255, 255 }}
            },
            .numPlataformas = 3,
            .numPerigos = 2,
            .numPortas = 2,
            // Come�am no canto superior esquerdo, na primeira plataforma
            .posInicialFogo = { 60, 240 },
            .posInicialAgua = { 100, 240 }
        }
    };
    int numTotalFases = sizeof(fases) / sizeof(fases[0]);
    int faseAtualIndex = 0;

    // Estado do Jogo
    EstadoJogo estadoJogo = JOGANDO;

    // Jogadores
    Jogador meninoFogo = { JOGADOR_FOGO, {0,0}, {0,0}, MAROON, false };
    Jogador meninaAgua = { JOGADOR_AGUA, {0,0}, {0,0}, BLUE, false };

    // Vetores da fase atual (ser�o preenchidos pela fun��o CarregarFase)
    Plataforma plataformasAtuais[MAX_PLATAFORMAS];
    Perigo perigosAtuais[MAX_PERIGOS];
    Porta portasAtuais[MAX_PORTAS];
    int numPlataformasAtuais, numPerigosAtuais, numPortasAtuais;

    // Carregar a primeira fase
    CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais, &numPerigosAtuais, portasAtuais, &numPortasAtuais);

    // Par�metros de f�sica
    const float gravidade = 0.10f;
    const float velocidadeMovimento = 4.0f;
    const float forcaPulo = -5.8f;

    // Vari�veis de estado para a vit�ria
    bool fogoNaPorta = false;
    bool aguaNaPorta = false;

    SetTargetFPS(60);

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // --- ATUALIZA��O ---
        switch (estadoJogo) {
            case JOGANDO: {
                // Controles dos jogadores (sem altera��o)
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
                // Sistema de Debug para pular de fase
                // printf("%d", GetKeyPressed());
                if (IsKeyPressed(332)) {
                    faseAtualIndex++; // Avan�a para a pr�xima fase
                    if (faseAtualIndex < numTotalFases) {
                        // Carrega a pr�xima fase
                        CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais, &numPerigosAtuais, portasAtuais, &numPortasAtuais);
                        estadoJogo = JOGANDO;
                }}


                // Atualizar f�sica e colis�es (usando os vetores da fase atual)
                AtualizarJogador(&meninoFogo, plataformasAtuais, numPlataformasAtuais, gravidade);
                AtualizarJogador(&meninaAgua, plataformasAtuais, numPlataformasAtuais, gravidade);

                // Verificar colis�es com perigos (usando os vetores da fase atual)
                for (int i = 0; i < numPerigosAtuais; i++) {
                    Rectangle retanguloJogadorFogo = { meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 };
                    Rectangle retanguloMeninaAgua = { meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 };

                    if (perigosAtuais[i].tipo == AGUA && CheckCollisionRecs(retanguloJogadorFogo, perigosAtuais[i].retangulo)) {
                        estadoJogo = FIM_DE_JOGO;
                    }
                    if (perigosAtuais[i].tipo == FOGO && CheckCollisionRecs(retanguloMeninaAgua, perigosAtuais[i].retangulo)) {
                        estadoJogo = FIM_DE_JOGO;
                    }
                }

                // Verificar condi��o de vit�ria (usando os vetores da fase atual)
                // � importante que portasAtuais[0] seja a porta do FOGO e portasAtuais[1] seja a da �GUA
                fogoNaPorta = CheckCollisionRecs((Rectangle){ meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 }, portasAtuais[0].retangulo);
                aguaNaPorta = CheckCollisionRecs((Rectangle){ meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 }, portasAtuais[1].retangulo);

                if (fogoNaPorta && aguaNaPorta) {
                    estadoJogo = VITORIA;
                }
            } break;

            case FIM_DE_JOGO: {
                if (IsKeyPressed(KEY_ENTER)) {
                    // Resetar a fase atual
                    CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais, &numPerigosAtuais, portasAtuais, &numPortasAtuais);
                    estadoJogo = JOGANDO;
                }
            } break;

            case VITORIA: {
                 if (IsKeyPressed(KEY_ENTER)) {
                    faseAtualIndex++; // Avan�a para a pr�xima fase
                    if (faseAtualIndex < numTotalFases) {
                        // Carrega a pr�xima fase
                        CarregarFase(fases[faseAtualIndex], &meninoFogo, &meninaAgua, plataformasAtuais, &numPlataformasAtuais, perigosAtuais, &numPerigosAtuais, portasAtuais, &numPortasAtuais);
                        estadoJogo = JOGANDO;
                    } else {
                        // Fim de jogo (ganhou todas as fases)
                        CloseWindow();
                    }
                }
            } break;
        }

        // --- DESENHO ---
        BeginDrawing();
            ClearBackground((Color){240, 240, 240, 255});

            // Desenhar elementos do n�vel
            for (int i = 0; i < numPlataformasAtuais; i++) DrawRectangleRec(plataformasAtuais[i].retangulo, DARKGRAY);
            for (int i = 0; i < numPerigosAtuais; i++) DrawRectangleRec(perigosAtuais[i].retangulo, perigosAtuais[i].cor);
            for (int i = 0; i < numPortasAtuais; i++) DrawRectangleRec(portasAtuais[i].retangulo, portasAtuais[i].cor);

            // Desenhar jogadores
            DrawRectangle(meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20, meninoFogo.cor);
            DrawRectangle(meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20, meninaAgua.cor);

            // Desenhar UI e mensagens de estado
            DrawText((TextFormat("Fase %d", faseAtualIndex + 1)), LARGURA_TELA - 100, 10, 20, LIGHTGRAY);
            if (estadoJogo == JOGANDO) {
                DrawText("Fogo: WASD | Agua: Setas", 10, 10, 20, DARKGRAY);
            } else if (estadoJogo == FIM_DE_JOGO) {
                DrawText("FIM DE JOGO", GetScreenWidth()/2 - MeasureText("FIM DE JOGO", 40)/2, GetScreenHeight()/2 - 40, 40, GRAY);
                DrawText("Pressione ENTER para reiniciar a fase", GetScreenWidth()/2 - MeasureText("Pressione ENTER para reiniciar a fase", 20)/2, GetScreenHeight()/2 + 10, 20, GRAY);
            } else if (estadoJogo == VITORIA) {
                DrawText("VITORIA!", GetScreenWidth()/2 - MeasureText("VITORIA!", 40)/2, GetScreenHeight()/2 - 40, 40, GOLD);
                if (faseAtualIndex + 1 < numTotalFases) {
                    DrawText("Pressione ENTER para a proxima fase", GetScreenWidth()/2 - MeasureText("Pressione ENTER para a proxima fase", 20)/2, GetScreenHeight()/2 + 10, 20, GOLD);
                } else {
                    DrawText("Parabens! Voce completou o jogo!", GetScreenWidth()/2 - MeasureText("Parabens! Voce completou o jogo!", 20)/2, GetScreenHeight()/2 + 10, 20, GOLD);
                }
            }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void CarregarFase(Fase fase, Jogador *meninoFogo, Jogador *meninaAgua, Plataforma plataformas[], int *numPlataformas, Perigo perigos[], int *numPerigos, Porta portas[], int *numPortas) {
    // Reseta a posi��o e velocidade dos jogadores
    meninoFogo->posicao = fase.posInicialFogo;
    meninaAgua->posicao = fase.posInicialAgua;
    meninoFogo->velocidade = (Vector2){0};
    meninaAgua->velocidade = (Vector2){0};

    // Copia os dados da estrutura da fase para as vari�veis de jogo
    *numPlataformas = fase.numPlataformas;
    for(int i = 0; i < fase.numPlataformas; i++) {
        plataformas[i] = fase.plataformas[i];
    }

    *numPerigos = fase.numPerigos;
    for(int i = 0; i < fase.numPerigos; i++) {
        perigos[i] = fase.perigos[i];
    }

    *numPortas = fase.numPortas;
    for(int i = 0; i < fase.numPortas; i++) {
        portas[i] = fase.portas[i];
    }
}

void AtualizarJogador(Jogador *jogador, Plataforma plataformas[], int numPlataformas, float gravidade) {
    // ... (c�digo da fun��o original sem altera��es)
    jogador->velocidade.y += gravidade;
    jogador->posicao.y += jogador->velocidade.y;

    float alturaJogador = 20.0f;
    Rectangle retanguloJogador = { jogador->posicao.x - 10, jogador->posicao.y - alturaJogador, 20, alturaJogador };

    jogador->podePular = false;
    for (int i = 0; i < numPlataformas; i++) {
        if (CheckCollisionRecs(retanguloJogador, plataformas[i].retangulo)) {
            if (jogador->velocidade.y > 0 && (retanguloJogador.y + retanguloJogador.height - jogador->velocidade.y) <= plataformas[i].retangulo.y) {
                jogador->posicao.y = plataformas[i].retangulo.y;
                jogador->velocidade.y = 0;
                jogador->podePular = true;
            }
            else if (jogador->velocidade.y < 0) {
                jogador->posicao.y = plataformas[i].retangulo.y + plataformas[i].retangulo.height + alturaJogador;
                jogador->velocidade.y = 0;
            }
        }
    }
}
