4.
  La clase de `Lock` indica cuál es el hilo que retiene el lock, siendo además
  el único que puede liberarlo. Un semáforo, en cambio, no garantiza que
  asignándole una prioridad mayor vaya a liberar su lugar con antelación, ya que
  cualquier otro semáforo puede realizar dicha operación.
