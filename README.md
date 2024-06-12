# philosofer-dinner
Progetto universitario: implementazione in c del famoso problema di mutua esclusione e gestione delle risorse "La cena dei filosofi", dove ogni filosofo ha due bacchette e il numero delle bacchette
è lo stesso dei filosofi. Qui maggiori informazioni: https://it.wikipedia.org/wiki/Problema_dei_filosofi_a_cena

# readme file

ESEGUIRE SCRIPT "run.sh" con 4 argomenti <br>

arg. 1 -> Quantità di filosofi <br>
arg. 2 -> se 1 rileva lo stallo <br>
arg. 3 -> se 1 evita lo stallo <br>
arg. 4 -> se 1 rileva la starvation <br>

leggenda: <br>

parent -> programma principale che esegue la cena dei filosofi <br>
controllore -> programma che agisce in background e controlla i filosofi di parent <br>
filelog.txt -> resoconto controllore <br>

# fine
