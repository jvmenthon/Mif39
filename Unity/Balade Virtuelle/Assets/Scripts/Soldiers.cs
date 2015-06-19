using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;


public class Soldiers: MonoBehaviour
{
	static private List<GameObject> soldiers = new List<GameObject> ();
	static bool isFirst = true;

	private List<Vector2> spawnPoints = new List<Vector2>{
		new Vector2(17f,45.6f),
		new Vector2(26f,45.6f),
		new Vector2(38f,37f),
		new Vector2(38f,25.5f),
		new Vector2(16.8f,23f),
		new Vector2(28f,13.7f),
		new Vector2(25.7f,-39.7f),
		new Vector2(-17f,-27f),
		new Vector2(-24f,-37f),
		new Vector2(-42f,-25f),
		new Vector2(-44.4f,26.5f),
		new Vector2(-26.5f,26.5f),
		new Vector2(-19.78f,39f),
		new Vector2(-37f,12.5f)
	};

	public Rigidbody rb;
	
	public float moveSpeed = 1f;
	
	// Use this for initialization
	void Start () {
		rb = GetComponent<Rigidbody> ();
		this.gameObject.name = "soldier";
		soldiers.Add (this.gameObject);
		if (isFirst) {
			isFirst = false;
			for (int i = 0; i<100; i++) {
				addSoldier ();
			}
		} else {
			moveOnStreet ();
		}
		
		//testSpawnPoint ();
	}

	void moveOnStreet(){
		//choix de la case
		int caseX = UnityEngine.Random.Range(-1,1);
		int caseZ = UnityEngine.Random.Range(-1,1);
		//choix du point de spawn
		Vector2 p = spawnPoints[UnityEngine.Random.Range (0,13)];
		//calcule de la position
		float x = p.x + caseX * Consts.CaseSize.x;
		float z = p.y + caseZ * Consts.CaseSize.y;
		Debug.Log (x + " " + z);
		transform.position = new Vector3 (x, 40.25f,z);
	}

	void addSoldier(){
		GameObject.Instantiate (this.gameObject);
	}
	
	// Update is called once per frame
	void Update () {
	}

	void testSpawnPoint(){
		for (int caseX=-1; caseX<=1; caseX++) {
			for (int caseZ=-1; caseZ<=1; caseZ++) {
				for (int i=0; i<spawnPoints.Count; i++) {
				
					//choix du point de spawn
					Vector2 p = spawnPoints [i];
					//calcule de la position
					float x = p.x + caseX * Consts.CaseSize.x;
					float z = p.y + caseZ * Consts.CaseSize.y;


					GameObject cube = GameObject.CreatePrimitive (PrimitiveType.Cube);
					cube.transform.position = new Vector3 (x, 55, z); 
					//cube.renderer.material.color = Color.green;
					}

			}
		}
	}
	/*
	public Soldiers (int nbSoliders)
	{
		soldiers = new List <Soldier> ();
		//Soldier s = new Soldier ();
		for (int i=0; i<10; i++) {
			soldiers.Add(new Soldier ());
		}
	}*/ 
}