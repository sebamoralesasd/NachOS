# Sebastián Morales - Parcial 1222

## Ejercicio 1
  - A y B: Administración de recursos. El sistema operativo (SO) es el encargado de
gestionar políticas para asignar recursos acorde a sus necesidades.
  - C: Abstracción. El SO organiza su información en archivos y directorios para
universalizar el acceso de los programas a la misma.
  - D: Aislamiento. El SO debe dar la ilusión de que la experiencia sea lo más
equitativa posible entre sus usuarios.

## Ejercicio 2
  - A: Falso. Si no se ha asociado ningún handler a una señal, se ejecuta el
handler por defecto.
  - B: Verdadero. `SIGKILL` es una señal que no puede ser ignorada o capturada.
  - C: Falso. `kill` es la función encargada de enviar una señal específica a un
proceso.

## Ejercicio 3

```C
int bool = 1;
static void sig_alrm(int signo) {
  bool = 0;
}

unsigned int sleep1(unsigned int nsecs) {
  signal(SIGALRM, sig_alrm);
  alarm(nsecs)
  while(bool);
  bool = 1;
  return(alarm(0));
}
```

## Ejercicio 4
  - a) Falso. `Thread::Sleep` pone el hilo en estado bloqueado.
  ```C
  status = BLOCKED;
  ```
  - b) Verdadero. `Thread::Yield` suspende el hilo llamante y elige a otro nuevo
para ejecutar. Sin embargo, si no hay otros hilos para dicha acción, se continúa
usando el hilo actual.
```C
Thread *nextThread = scheduler->FindNextToRun();
 if (nextThread != nullptr) {
     scheduler->ReadyToRun(this);
     scheduler->Run(nextThread);
 }
```
  - c) Falso. Un hilo continúa su ejecución si no hay otro (hilo) listo.
  - d)
  - e) 

## Ejercicio 5

| Tick | Proceso | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 |
|------------|---------|---|---|---|----|-----|-----|----|----|----|---|----|----|
| **Nuevos** | A | 0 | 2 | 4 | 6 | 8 | 10 |  |  |  |  |  |  |
|  | B | 2 | 4 | 6 | 8 |  |  |  |  |  |  |  |  |
| **Aceptados** | C | 5 | 6 | 7 | 8 | 9 | 10 |  |  |  |  |  |  |
|  | D | 7 | 8 | 9 | 10 | 11 | 12 |  |  |  |  |  |  |
|  | B |  |  |  |  | 9 | 10 |  |  |  |  |  |  |
| **Estado** |  | D | C | D | BC |  CD | DAB | AB | BD | DA | A |  |  |
| **Ejecutando** |  | C | D | C | D | B | C | D | A | B | D | A |  |
| **Terminó** |  |  |  |  |  |  |  | C |  |  | B | D | A |

| Proceso | Tiempo llegada | t | inicio | Fin | T | E | P | R |
|----------|----------------|------|--------|-----|----|------|-------|--------|
| A | 0 | 2 | 7 | 11 | 11 | 9 | 5.5 | 2/11 |
| B | 0 | 2 | 4 | 9 | 9 | 7 | 4.5 | 2/9 |
| C | 0 | 3 | 0 | 6 | 6 | 3 | 2 | 0.5 |
| D | 0 | 4 | 1 | 10 | 10 | 6 | 2.5 | 0.4 |
| **Promedio** |  | 2.75 |  |  | 9 | 6.25 | 3.625 | 4/14.5 |
