# Mif39

A faire : 
modifier le CMakeLists.txt principal pour spécifier le chemin d'accès vers QT


## Probleme
L'environnement est modifiable uniquement par le thread princpal, il va falloir trouver un mecanisme pour transferer les données a ce thread


## Qu'est-ce qu'on fait ?

Développement d'un environnement dans lequel évoluent des objets au comportement autonome
Fonctionne sous une architecture client/serveur: 
Du côté serveur se trouve le monde, les clients s'y connectent avec un objet 3D

A l'ouverture, le serveur ouvre le monde. Les clients communiquent avec le serveur pour le mettre a jour.
Les clients donnent les informations de mise a jour au serveur en cas de mouvement ... etc, dans ce cas, le serveur récupère les nouvelles 
données et broadcast chez tous les clients.

Le monde doit être découpé en petites cases comportant des informations. Quand il se connecte au monde, le client reçoit les informations
des cases se trouvant proche du client pour ne pas qu'il ai a charger le monde enier

Copier les descriptions des mime type (les 2 fichiers xml dans le dossier Resources)
- linux : dans le dossier ~/.local/share/mime/packages
- macos : dans le dossier ~/Library/Application Support/mime/packages
- windows : dans le dossier C:/Users/<USER>/AppData/Local/mime/packages
