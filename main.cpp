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
#include "Source.h"
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

Color getColorAt(Vect intersection_position, Vect intersecting_ray_direction, vector<Object*> scene_objects, int index_of_winning_object, vector<Source*> light_sources, double accuracy, double ambientlight){

    Color winning_object_color = scene_objects.at(index_of_winning_object)->getColor();
    Vect winning_object_normal = scene_objects.at(index_of_winning_object)->getNormalAt(intersection_position);

    Color final_color = winning_object_color.colorScalar(ambientlight);

    for (int light_index = 0; light_index < light_sources.size(); light_index++){
        Vect light_direction = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
        
        float cosine_angle = winning_object_normal.dot(light_direction);

        if (cosine_angle > 0){
            bool shadowed = false;

            Vect distance_to_light = light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()).normalize();
            float distance_to_light_magnitude = distance_to_light.magnitude();

            Ray shadow_ray(intersection_position, light_sources.at(light_index)->getLightPosition().vectAdd(intersection_position.negative()));

            vector<double> secondary_intersections;

            for (int object_index = 0; object_index < scene_objects.size() && shadowed ==false; object_index++){
                secondary_intersections.push_back(scene_objects.at(object_index)->findIntersection(shadow_ray));
            }

            for (int c = 0; c < secondary_intersections.size(); c++){
                if (secondary_intersections.at(c) > accuracy){
                    if (secondary_intersections.at(c) <= distance_to_light_magnitude){
                        shadowed = true;
                    }
                    break;
                }
                if (shadowed == false){
                    final_color = final_color.colorAdd(winning_object_color.colorMultiply(light_sources.at(light_index)->getLightColor()).colorScalar(cosine_angle));

                    if (winning_object_color.getColorSpecial()> 0 && winning_object_color.getColorSpecial() <=1){
                        double dot1 = winning_object_normal.dot(intersecting_ray_direction.negative());
                        Vect scalar1 = winning_object_normal.vectMult(dot1);
                        Vect add1 = scalar1.vectAdd(intersecting_ray_direction);
                        Vect scalar2 = add1.vectMult(2);
                        Vect add2 = intersecting_ray_direction.negative().vectAdd(scalar2);
                        Vect reflection_direction = add2.normalize();

                        double specular = reflection_direction.dot(light_direction);
                        if (specular > 0){
                            specular = pow(specular, 10);
                            final_color = final_color.colorAdd(light_sources.at(light_index)->getLightColor().colorScalar(specular * winning_object_color.getColorSpecial()));
                        }
                    }
                }
            }
        }
    }
        return final_color.clip();
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

        

        int width = 1280;
        int height = 720;
        int n = width * height;
        double aspectratio = (double)width / (double)height;
        double ambientlight = 0.2f;
        double accuracy = 0.000001;

        Vect O(0.0f, 0.0f, -6.0f);
        Vect X(1, 0, 0);
        Vect Y(0, 1, 0);
        Vect Z(0, 0, 1);

        Vect campos(0.0f, 0.0f, 0.0f);
        Vect look_at(0, 0, -10);
        Vect diff_btw(campos.getVectX() - look_at.getVectX(), campos.getVectY() - look_at.getVectY(), campos.getVectZ() - look_at.getVectZ());

        Vect camdir = diff_btw.negative().normalize();
        Vect camright = Y.crossProduct(camdir).normalize();
        Vect camdown = camright.crossProduct(camdir);
        Camera scene_cam(campos, camdir, camright, camdown);

        Color white(1.0, 1.0, 1.0, 0);
        Color green(0.5, 1.0, 0.5, 0.5);
        Color gray(0.5, 0.5, 0.5, 0);
        Color black(0.0, 0.0, 0.0, 0);
        Color red(1.0, 0, 0, 0);

        Vect light_position(0.0f, 5.0f, 0.0f);
        Light scene_light(light_position, white);
        vector<Source*> light_sources;
        light_sources.push_back(dynamic_cast<Source*>(&scene_light));

        Sphere scene_sphere(O, 1, red);
        // Plane scene_plane(Y, -1, gray);

        vector<Object*> scene_objects;
        scene_objects.push_back(&scene_sphere);
        // scene_objects.push_back(&scene_plane);

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
                    if(intersections.at(index_of_winning_object) > accuracy){

                        Vect intersection_position = cam_ray_origin.vectAdd(cam_ray_direction.vectMult(intersections.at(index_of_winning_object)));
                        Vect intersecting_ray_direction = cam_ray_direction;


                        Color intersection_color = getColorAt(intersection_position, intersecting_ray_direction, scene_objects, index_of_winning_object, light_sources, accuracy, ambientlight);
                        double r = intersection_color.getColorRed() * 255;
                        double g = intersection_color.getColorGreen() * 255;
                        double b = intersection_color.getColorBlue() * 255;
                        SDL_SetRenderDrawColor(renderer, r, g, b, 255); 
                        SDL_RenderDrawPoint(renderer, x, y); //<---- Aqui escolhemos qual pixel vamos pintar
                    }
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