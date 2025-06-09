#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>

#define LARGURA_TELA 800
#define ALTURA_TELA 600

// --- Estruturas de Dados ---

typedef enum { JOGANDO, FIM_DE_JOGO, VITORIA } EstadoJogo;
typedef enum { FOGO, AGUA } TipoPerigo;
typedef enum { JOGADOR_FOGO, JOGADOR_AGUA } TipoJogador;

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

// --- Funções de Ajuda ---
void AtualizarJogador(Jogador *jogador, Plataforma plataformas[], int numPlataformas, float gravidade);

// --- Função Principal ---
int main(void) {
    // Inicialização
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Menino Fogo e Menina Água - Nível Ajustado");

    // Estado do Jogo
    EstadoJogo estadoJogo = JOGANDO;

    // --- LINHAS CORRIGIDAS ---
    Jogador meninoFogo = { JOGADOR_FOGO }; // Corrigido para evitar conversão inválida
    meninoFogo.tipo = JOGADOR_FOGO;
    meninoFogo.posicao = (Vector2){ 80, 550 };
    meninoFogo.cor = MAROON;
    meninoFogo.podePular = false;

    Jogador meninaAgua = { JOGADOR_AGUA }; // Corrigido para evitar conversão inválida
    meninaAgua.tipo = JOGADOR_AGUA;
    meninaAgua.posicao = (Vector2){ LARGURA_TELA - 80, 550 };
    meninaAgua.cor = BLUE;
    meninaAgua.podePular = false;


    // --- LEVEL DESIGN AJUSTADO PARA SER POSSÍVEL ---
    Plataforma plataformas[] = {
        {{ 0, 550, LARGURA_TELA, 50 }},      // Chão
        {{ 50, 480, 120, 20 }},             // 1. Esquerda-Baixo (pulo de 70px de altura)
        {{ LARGURA_TELA - 170, 480, 120, 20 }}, // 1. Direita-Baixo
        {{ 200, 410, 120, 20 }},            // 2. Esquerda-Meio (pulo de 70px de altura)
        {{ LARGURA_TELA - 320, 410, 120, 20 }}, // 2. Direita-Meio
        {{ 50, 340, 120, 20 }},             // 3. Esquerda-Alto (pulo de 70px de altura)
        {{ LARGURA_TELA - 170, 340, 120, 20 }}, // 3. Direita-Alto
        {{ 250, 280, 300, 20 }},            // 4. Plataforma Central, para encontro
        {{ 325, 230, 150, 20 }}              // 5. Plataforma Final (MAIS BAIXA E MAIS LARGA)
    };
    int numPlataformas = sizeof(plataformas)/sizeof(plataformas[0]);

    Perigo perigos[] = {
        {{ 250, 530, 100, 20 }, FOGO, RED},
        {{ LARGURA_TELA - 350, 530, 100, 20 }, AGUA, SKYBLUE}
    };
    int numPerigos = sizeof(perigos)/sizeof(perigos[0]);

    Porta portas[] = {
        {{ 360, 190, 40, 40 }, JOGADOR_FOGO, (Color){ 255, 100, 100, 255 }},
        {{ 410, 190, 40, 40 }, JOGADOR_AGUA, (Color){ 100, 100, 255, 255 }}
    };
    int numPortas = sizeof(portas)/sizeof(portas[0]);
    // --- FIM DO LEVEL DESIGN ---


    // --- PARÂMETROS DE DIFICULDADE (PULO MENOR) ---
    const float gravidade = 0.7f;
    const float velocidadeMovimento = 4.0f;
    const float forcaPulo = -11.0f;
    // --- FIM DOS PARÂMETROS DE DIFICULDADE ---


    // Variáveis de estado para a vitória
    bool fogoNaPorta = false;
    bool aguaNaPorta = false;

    SetTargetFPS(60);

    // Loop principal do jogo
    while (!WindowShouldClose()) {
        // --- ATUALIZAÇÃO ---

        switch (estadoJogo) {
            case JOGANDO: {
                // Controles Menino Fogo (WASD)
                if (IsKeyDown(KEY_A)) meninoFogo.posicao.x -= velocidadeMovimento;
                if (IsKeyDown(KEY_D)) meninoFogo.posicao.x += velocidadeMovimento;
                if (IsKeyPressed(KEY_W) && meninoFogo.podePular) {
                    meninoFogo.velocidade.y = forcaPulo;
                    meninoFogo.podePular = false;
                }

                // Controles Menina Água (Setas)
                if (IsKeyDown(KEY_LEFT)) meninaAgua.posicao.x -= velocidadeMovimento;
                if (IsKeyDown(KEY_RIGHT)) meninaAgua.posicao.x += velocidadeMovimento;
                if (IsKeyPressed(KEY_UP) && meninaAgua.podePular) {
                    meninaAgua.velocidade.y = forcaPulo;
                    meninaAgua.podePular = false;
                }

                // Atualizar física e colisões dos jogadores
                AtualizarJogador(&meninoFogo, plataformas, numPlataformas, gravidade);
                AtualizarJogador(&meninaAgua, plataformas, numPlataformas, gravidade);

                // Verificar colisões com perigos
                for (int i = 0; i < numPerigos; i++) {
                    Rectangle retanguloJogadorFogo = { meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 };
                    Rectangle retanguloMeninaAgua = { meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 };

                    if (perigos[i].tipo == AGUA && CheckCollisionRecs(retanguloJogadorFogo, perigos[i].retangulo)) {
                        estadoJogo = FIM_DE_JOGO;
                    }
                    if (perigos[i].tipo == FOGO && CheckCollisionRecs(retanguloMeninaAgua, perigos[i].retangulo)) {
                        estadoJogo = FIM_DE_JOGO;
                    }
                }

                // Verificar condição de vitória
                fogoNaPorta = CheckCollisionRecs((Rectangle){ meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20 }, portas[0].retangulo);
                aguaNaPorta = CheckCollisionRecs((Rectangle){ meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20 }, portas[1].retangulo);

                if (fogoNaPorta && aguaNaPorta) {
                    estadoJogo = VITORIA;
                }

            } break;

            case FIM_DE_JOGO: {
                if (IsKeyPressed(KEY_ENTER)) {
                    // Resetar jogo
                    meninoFogo.posicao = (Vector2){ 80, 550 };
                    meninaAgua.posicao = (Vector2){ LARGURA_TELA - 80, 550 };
                    meninoFogo.velocidade = (Vector2){0};
                    meninaAgua.velocidade = (Vector2){0};
                    estadoJogo = JOGANDO;
                }
            } break;

            case VITORIA: {
                 if (IsKeyPressed(KEY_ENTER)) {
                    // Resetar jogo (ou ir para próximo nível)
                    meninoFogo.posicao = (Vector2){ 80, 550 };
                    meninaAgua.posicao = (Vector2){ LARGURA_TELA - 80, 550 };
                    meninoFogo.velocidade = (Vector2){0};
                    meninaAgua.velocidade = (Vector2){0};
                    estadoJogo = JOGANDO;
                }
            } break;
        }

        // --- DESENHO ---
        BeginDrawing();
            ClearBackground((Color){240, 240, 240, 255}); // Fundo cinza claro

            // Desenhar elementos do nível
            for (int i = 0; i < numPlataformas; i++) DrawRectangleRec(plataformas[i].retangulo, DARKGRAY);
            for (int i = 0; i < numPerigos; i++) DrawRectangleRec(perigos[i].retangulo, perigos[i].cor);
            for (int i = 0; i < numPortas; i++) DrawRectangleRec(portas[i].retangulo, portas[i].cor);

            // Desenhar jogadores
            DrawRectangle(meninoFogo.posicao.x - 10, meninoFogo.posicao.y - 20, 20, 20, meninoFogo.cor);
            DrawRectangle(meninaAgua.posicao.x - 10, meninaAgua.posicao.y - 20, 20, 20, meninaAgua.cor);

            // Desenhar UI e mensagens de estado
            if (estadoJogo == JOGANDO) {
                DrawText("Fogo: WASD | Agua: Setas", 10, 10, 20, DARKGRAY);
            } else if (estadoJogo == FIM_DE_JOGO) {
                DrawText("FIM DE JOGO", GetScreenWidth()/2 - MeasureText("FIM DE JOGO", 40)/2, GetScreenHeight()/2 - 40, 40, GRAY);
                DrawText("Pressione ENTER para reiniciar", GetScreenWidth()/2 - MeasureText("Pressione ENTER para reiniciar", 20)/2, GetScreenHeight()/2 + 10, 20, GRAY);
            } else if (estadoJogo == VITORIA) {
                DrawText("VITORIA!", GetScreenWidth()/2 - MeasureText("VITORIA!", 40)/2, GetScreenHeight()/2 - 40, 40, GOLD);
                DrawText("Pressione ENTER para reiniciar", GetScreenWidth()/2 - MeasureText("Pressione ENTER para reiniciar", 20)/2, GetScreenHeight()/2 + 10, 20, GOLD);
            }


        EndDrawing();
    }

    // Finalização
    CloseWindow();
    return 0;
}


// Função de atualização do jogador (sem alterações)
void AtualizarJogador(Jogador *jogador, Plataforma plataformas[], int numPlataformas, float gravidade) {
    // Aplicar gravidade e atualizar posição Y
    jogador->velocidade.y += gravidade;
    jogador->posicao.y += jogador->velocidade.y;

    // A altura do jogador é 20. Sua posição Y é a base do retângulo (pés).
    float alturaJogador = 20.0f;
    Rectangle retanguloJogador = { jogador->posicao.x - 10, jogador->posicao.y - alturaJogador, 20, alturaJogador };

    // Colisão com as plataformas
    jogador->podePular = false;
    for (int i = 0; i < numPlataformas; i++) {
        if (CheckCollisionRecs(retanguloJogador, plataformas[i].retangulo)) {
            // Verifica colisão EM CIMA da plataforma (caindo)
            if (jogador->velocidade.y > 0 && (retanguloJogador.y + retanguloJogador.height - jogador->velocidade.y) <= plataformas[i].retangulo.y) {
                // A posição Y (pés) do jogador deve ser igual à posição Y (topo) da plataforma.
                jogador->posicao.y = plataformas[i].retangulo.y;
                jogador->velocidade.y = 0;
                jogador->podePular = true;
            }
            // Verifica colisão EM BAIXO da plataforma (pulando)
            else if (jogador->velocidade.y < 0) {
                // Ajusta a posição para ficar exatamente embaixo da plataforma, impedindo que atravesse
                jogador->posicao.y = plataformas[i].retangulo.y + plataformas[i].retangulo.height + alturaJogador;
                jogador->velocidade.y = 0; // Para o movimento vertical, fazendo o jogador "bater a cabeça" e começar a cair
            }
        }
    }
}
