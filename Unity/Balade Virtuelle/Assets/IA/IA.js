#pragma strict

var cible : GameObject;
var balle : GameObject;
var faireFeu;

function Start () {

}

function Update () {

	var rotate = Quaternion.LookRotation(transform.position - cible.transform.position);
	//Fait comme la ligne d'en dessous mais avec un petit décallage dans le temps, donc désactivé pour les tests
	//transform.rotation = Quaternion.Slerp(transform.rotation, rotate, Time.deltaTime * 2);
	transform.rotation = rotate; 
	
	//Mise au point du canon, car petit décallage, Quaternion.Slerp devra etre appliqué aussi ici
	var rotation = Quaternion.LookRotation(cible.transform.position);
 	//rotation *= Quaternion.Euler(0, 90, 90);
 	transform.rotation = rotation;
	
	
	var temps : int = Time.time;
	var cadence = (temps % 4);
	if(cadence) {
		tir(temps);
	}
}

function tir(temps) {

	if(temps != faireFeu) {
		var tirer = Instantiate(balle, transform.Find("depart").transform.position, Quaternion.identity);
		tirer.GetComponent.<Rigidbody>().AddForce(transform.forward * 1000);
		faireFeu = temps;
	}

}