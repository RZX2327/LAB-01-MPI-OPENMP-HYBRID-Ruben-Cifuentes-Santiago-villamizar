# LAB-01-MPI-OPENMP-HYBRID-Ruben-Cifuentes-Santiago-villamizar
## Ejercicio 1 — Hola Mundo MPI
## Cada proceso MPI debe imprimirse a sí mismo: su rank (ID) y el total de procesos. Adicionalmente, el proceso con rank 0 actuará como 'maestro' e imprimirá un mensaje especial.

<img width="350" height="186" alt="image" src="https://github.com/user-attachments/assets/395efed0-572e-46fc-a3c3-9fe7ccf863b4" />

<img width="349" height="152" alt="image" src="https://github.com/user-attachments/assets/317b35e9-bdcf-4778-9605-90c8b53d91f4" />

##¿Por qué varía el orden de salida? Porque los procesos MPI corren en paralelo real — cada uno es un proceso independiente del 
sistema operativo. El orden en que imprimen depende del scheduler del SO, la carga del CPU y la velocidad de cada proceso. No hay garantía de orden a menos que uses sincronización explícita (MPI_Barrier, mensajes, etc.). Por eso ves que el proceso 0 a veces sale antes que el 1, 2 o 3 — o después.

¿Qué pasa con -n 1? Con un solo proceso, world_size = 1 y world_rank = 0, así que ese único proceso es a la vez maestro y el único trabajador. El programa funciona, pero no hay paralelismo real — es como ejecutarlo sin MPI. No tiene sentido paralelizar así porque todo el trabajo recae en un solo núcleo, perdiendo el propósito de MPI que está diseñado para distribuir trabajo entre múltiples procesos.

¿Para qué sirve MPI_COMM_WORLD? Es el comunicador global por defecto — agrupa automáticamente a todos los procesos lanzados con mpiexec. Define el "universo" de comunicación: quiénes pueden enviarse mensajes entre sí. Sí pueden existir otros comunicadores. Se crean con funciones como MPI_Comm_split o MPI_Comm_create, y sirven para dividir los procesos en subgrupos. Por ejemplo, si tienes 8 procesos puedes crear un comunicador solo con los pares (0, 2, 4, 6) y otro con los impares (1, 3, 5, 7) para coordinar operaciones independientes dentro de cada grupo.

## Ejercicio 2 — OpenMP dentro de MPI
## MPI distribuye el trabajo entre procesos (memoria distribuida). Dentro de cada proceso, OpenMP distribuye el trabajo entre hilos (memoria compartida). La combinación se llama programación híbrida MPI+OpenMP.

<img width="612" height="249" alt="image" src="https://github.com/user-attachments/assets/862f98e2-e6e7-41ea-a6d7-8d8eb8479027" />

<img width="617" height="392" alt="image" src="https://github.com/user-attachments/assets/3be73cc4-0586-44a5-8b4e-301a91b3b930" />

¿Cuántas unidades con 2 procesos MPI y 4 hilos OMP? 2 × 4 = 8 unidades de cómputo. Cada proceso MPI crea 4 hilos internamente, todos corriendo en paralelo.

¿Diferencia entre -n 4 (4 MPI, 4 hilos) vs -n 1 (1 MPI, 16 hilos)? Ambos dan 16 unidades totales, pero son modelos distintos. Con -n 4 la memoria está distribuida — cada proceso tiene su propio espacio de memoria y se comunican por mensajes, lo que escala bien en múltiples máquinas. Con -n 1 toda la memoria es compartida entre los 16 hilos, sin overhead de comunicación, pero limitado a una sola máquina. En la práctica -n 4 es mejor para clusters; -n 1 para un solo nodo potente.

¿Por qué MPI_Init_thread y no MPI_Init? MPI_Init asume que solo habrá un hilo haciendo llamadas MPI. Al mezclar con OpenMP, varios hilos podrían llamar a MPI simultáneamente y causar condiciones de carrera o comportamiento indefinido. MPI_Init_thread le indica a la librería el nivel de soporte necesario — en este caso MPI_THREAD_FUNNELED garantiza que solo el hilo master (el que llamó a MPI_Init_thread) haga llamadas MPI, haciendo la ejecución segura.

## Ejercicio 3 — Suma Híbrida
El proceso maestro (rank 0) inicializa un vector de N=1,000,000 enteros, donde cada elemento vale su índice (arr[i] = i). Se debe calcular la suma total de todos los elementos usando MPI_Scatter para distribuir secciones del vector y OpenMP para sumar cada sección en paralelo.
<img width="685" height="132" alt="image" src="https://github.com/user-attachments/assets/fede2a32-e254-4b94-8705-3ed0b99e8ce3" />
¿Qué hace MPI_Scatter? Toma el arreglo completo que tiene rank 0 y lo divide en size trozos iguales, enviando un trozo distinto a cada proceso. Rank 0 es el único que envía — los demás solo reciben. Con N=1000000 y 4 procesos, cada uno recibe 250,000 elementos: rank 0 recibe [0..249999], rank 1 recibe [250000..499999], y así sucesivamente. Es la operación inversa de MPI_Gather.

¿Por qué reduction(+:suma_local) y no variable compartida? Con variable compartida, varios hilos leerían y escribirían suma_local al mismo tiempo causando una condición de carrera — el resultado sería incorrecto e impredecible. La cláusula reduction hace que cada hilo tenga su propia copia privada, las suma todas de forma segura al final de la región paralela, y deposita el resultado en la variable original. Es la forma correcta y eficiente de acumular en paralelo.

¿Qué pasaría si olvidaras MPI_Reduce? suma_local en rank 0 solo contendría la suma de su propio trozo ([0..249999]), que es 31249875000 — solo 1/4 del resultado correcto. Los demás procesos habrían calculado su parte pero nadie la recogería. El programa imprimiría un valor incorrecto sin ningún error visible, que es el peor tipo de bug en cómputo paralelo.

## Ejercicio 4 (Reto) — Speedup
Partiendo del Ejercicio 3, modificarás el programa para medir tres versiones de la suma y calcular el speedup de cada una.
<img width="692" height="128" alt="image" src="https://github.com/user-attachments/assets/44e4499b-dab0-4402-aa20-27eec2557754" />
¿Qué hace MPI_Scatter? Toma el arreglo completo que tiene rank 0 y lo divide en size trozos iguales, enviando un trozo distinto a cada proceso. Rank 0 es el único que envía — los demás solo reciben. Con N=1000000 y 4 procesos, cada uno recibe 250,000 elementos: rank 0 recibe [0..249999], rank 1 recibe [250000..499999], y así sucesivamente. Es la operación inversa de MPI_Gather.

¿Por qué reduction(+:suma_local) y no variable compartida? Con variable compartida, varios hilos leerían y escribirían suma_local al mismo tiempo causando una condición de carrera — el resultado sería incorrecto e impredecible. La cláusula reduction hace que cada hilo tenga su propia copia privada, las suma todas de forma segura al final de la región paralela, y deposita el resultado en la variable original. Es la forma correcta y eficiente de acumular en paralelo.

¿Qué pasaría si olvidaras MPI_Reduce? suma_local en rank 0 solo contendría la suma de su propio trozo ([0..249999]), que es 31249875000 — solo 1/4 del resultado correcto. Los demás procesos habrían calculado su parte pero nadie la recogería. El programa imprimiría un valor incorrecto sin ningún error visible, que es el peor tipo de bug en cómputo paralelo.
