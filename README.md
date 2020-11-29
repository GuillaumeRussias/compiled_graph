# Compiled Graph
Petite bibilothèque c++ interfacée avec python pour la gestion de graph.

# Installation :
* (1) **Avec Git Bash**  : ouvrir Git Bash dans le répertoire dans lequel vous voulez téléchargez le projet:
  git clone https://github.com/GuillaumeRussias/compiled_graph

* (1) **Sans Git Bash** : Téléchargez ce projet manuellement.
 
* (2) **Sur windows** : télécharger au préalable le compilateur msvc : https://visualstudio.microsoft.com/fr/visual-cpp-build-tools/ (laisser toutes les instructions par défaut).

* (2) **Sur mac et linux** : aller directement étape (3). (testé sur linux , supposé pour mac).

* (3) **Avec anaconda** : AnacondaPrompt -> activer un environnement -> aller dans le répertoire **parent** du projet -> pip install ./compiled_graph  .

* (3) **Avec un bash**  : bash -> aller dans le répertoire du projet -> python -m pip install ./compiled_graph .

# Tester :
1. Vérifier que l'environnement dans lequel vous avez installer la bibliothèque possède numpy. sinon : conda install numpy ou pip install numpy .

2. Exécuter test_compiled_graph.py . S'il n'ya pas d'erreur , l'installation fonctionne.


# Credits :
setup.py : Sylvain Corlay
