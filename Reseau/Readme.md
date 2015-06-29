## Makefile
make : compile cette partie Réseau
make clean : supprimer les fichiers temporaires

## Mise en place des tests:
1. make clean
2. make
3. ./run.sh (plusieur options à choisir)

## Exemple (vu dans compte rendu):
1. Création 3 noeuds, attacher 3 clien ControlUI :
	./run.sh d 3
	./run.sh c 1
	./run.sh c 2
	./run.sh c 3
Sur les fenêtres client, taper "help" pour accéder à l'aide
	
2. Mise en connexion nœud 1 et nœud 3 (sur client ui 3)
	join 127.0.0.1:10002:10003
3. Depuis nœud 3 : recherche le fichier hello.txt
	search hello.txt
Les recherches et les résultats peuvent ensuite affichés dans list_search ou list_result [numéro de la recherche]
	list_search
	list_search 0
4. Télécharge le fichier hello.txt
	get 0 0
5. Même procédure pour le fichier image test.jpg sur le nœud 2
6. Vérification si nous avons bien 2 fichiers en local pour le nœud 3 et si l'image s'affiche bien

## Vidéo démo
https://www.youtube.com/watch?v=2UM61SByFVE
