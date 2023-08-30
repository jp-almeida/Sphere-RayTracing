#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <limits>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "src/include/SDL2/SDL.h"
#include "Vect.h"
#include "Ray.h"
#include "Camera.h"
#include "Color.h"
#include "Light.h"
#include "Sphere.h"
#include "Object.h"
#include "Plane.h"

using namespace std;

int winningObjectIndex(vector<double> object_intersections){
    int index_of_minimum_value;
    
    if(object_intersections.size() == 0){
        return -1;
    }else if (object_intersections.size() == 1){
        if (object_intersections.at(0) > 0){
            return 0;
        }else{
            return -1;
        }
    }else{
        double max = 0;
        for (int i = 0; i < object_intersections.size(); i++){
            if (max < object_intersections.at(i)){
                max = object_intersections.at(i);
            }
        }
        if (max > 0){
            for (int index = 0; index < object_intersections.size(); index++){
                if (object_intersections.at(index) > 0 && object_intersections.at(index) <= max){
                    max = object_intersections.at(index);
                    index_of_minimum_value = index;
                }
            }
            return index_of_minimum_value;
        }else{
            return -1;
        }
    }
}

int main(int argc, char* argv[]) {

    // Inicializar a biblioteca para poder usar suas funções
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Não foi possível inicializar o SDL! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    // Criar uma janela
    SDL_Window* window = SDL_CreateWindow(
        "CG I - Raycaster",       // Título da Janela
        SDL_WINDOWPOS_UNDEFINED,        // Posição inicial X
        SDL_WINDOWPOS_UNDEFINED,        // Posição inicial Y
        1280,                           // Largura da janela em pixels
        720,                            // Altura da janela em pixels
        SDL_WINDOW_SHOWN                // Flags
    );

    // Verificar se a janela foi criada corretamente
    if (!window) {
        SDL_Log("Criação da janela falhou! SDL_Error: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Criar o objeto renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Criação do renderer falhou! SDL_Error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop - É dentro do main while loop que fazemos todas as interações com a janela
    bool isRunning = true; //variável de controle do loop
    SDL_Event event; //variável para checar os eventos da janela

    while (isRunning) {
        // É possível interagir com a janela, com clicks, apertando teclas ou clicando para fechar a janela
        // essas interações são chamadas de "eventos", abaixo, verificamos se o evento de fechar janela ocorre
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        // É uma boa prática limpar o renderer antes de desenhar novos pixeis, no caso de existir "lixo" no renderer
        SDL_RenderClear(renderer);

        // Aqui temos a estrutura para pintar um pixel, no caso, um loop para pintar todos os pixeis da janela

        

        int width = 640;
        int height = 480;
        int n = width * height;
        double aspectratio = (double)width / (double)height;

        Vect O(0, 0, 0);
        Vect X(1, 0, 0);
        Vect Y(0, 1, 0);
        Vect Z(0, 0, 1);

        Vect campos(3, 1.5, -4);
        Vect look_at(0, 0, 0);
        Vect diff_btw(campos.getVectX() - look_at.getVectX(), campos.getVectY() - look_at.getVectY(), campos.getVectZ() - look_at.getVectZ());

        Vect camdir = diff_btw.negative().normalize();
        Vect camright = Y.crossProduct(camdir).normalize();
        Vect camdown = camright.crossProduct(camdir);
        Camera scene_cam(campos, camdir, camright, camdown);

        Color white(1.0, 1.0, 1.0, 0);
        Color green(0.5, 1.0, 0.5, 0.5);
        Color gray(0.5, 0.5, 0.5, 0);
        Color black(0.0, 0.0, 0.0, 0);

        Vect light_position(-7, 10, -10);
        Light scene_light(light_position, white);

        Sphere scene_sphere(O, 1, green);
        Plane scene_plane(Y, -1, white);

        vector<Object*> scene_objects;
        scene_objects.push_back(&scene_sphere);
        scene_objects.push_back(&scene_plane);

        double xamnt, yamnt;

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                if(width > height){
                    xamnt = ((x + 0.5) / width) * aspectratio - ((width - height) / (double)height / 2);
                    yamnt = ((height - y) + 0.5) / height;
                }else if (height > width){
                    xamnt = (x + 0.5) / width;
                    yamnt = (((height - y) + 0.5) / height) / aspectratio - ((height - width) / double(width) / 2);
                }else{
                    xamnt = (x + 0.5) / width;
                    yamnt = ((height - y) + 0.5) / height;
                }

                Vect cam_ray_origin = scene_cam.getCameraPosition();
                Vect cam_ray_direction = camdir.vectAdd(camright.vectMult(xamnt - 0.5).vectAdd(camdown.vectMult(yamnt - 0.5))).normalize();

                Ray cam_ray(cam_ray_origin, cam_ray_direction);

                vector<double> intersections;

                for (int index = 0; index < scene_objects.size();index++){
                    intersections.push_back(scene_objects[index]->findIntersection(cam_ray));
                }

                int index_of_winning_object = winningObjectIndex(intersections);

                if (index_of_winning_object == -1){
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
                    SDL_RenderDrawPoint(renderer, x, y); //<---- Aqui escolhemos qual pixel vamos pintar
                }else{
                    Color this_color = scene_objects.at(index_of_winning_object)->getColor();
                    double r = this_color.getColorRed() * 255;
                    double g = this_color.getColorGreen() * 255;
                    double b = this_color.getColorBlue() * 255;
                    SDL_SetRenderDrawColor(renderer, r, g, b, 255); 
                    SDL_RenderDrawPoint(renderer, x, y); //<---- Aqui escolhemos qual pixel vamos pintar
                }

                
            }
        }

        // Por fim, atualizamos a janela com o renderer que acabamos de pintar e tudo deve funcionar corretamente
        SDL_RenderPresent(renderer);
    }

    // Destruir os objetos criados para limpar a memória
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}