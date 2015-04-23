using UnityEngine;
using System.Collections;

public class CameraController : MonoBehaviour {
	
	public GameObject player;
	public Camera camera;
	private Vector3 offset;
	private float turnSpeed = 5.0f;
	private int zoom = 5;
	private int normal = 60;
	private int cpt = 0;
	private float smooth = 5.0f;
	private bool isZoomed = false;
	private bool isDezoomed = false;

	
	// Use this for initialization
	void Start () {

		offset = transform.position - player.transform.position;
	}

	void Update(){
		const int orthographicSizeMin = 1; const int orthographicSizeMax = 6;

		if(Input.GetAxis("Mouse ScrollWheel")>0){ isZoomed = true; isDezoomed = false;
			Camera.main.orthographicSize++;}
		else if (Input.GetAxis ("Mouse ScrollWheel") < 0) {
			isDezoomed = true;
			isZoomed = false;
			Camera.main.orthographicSize--;
		} 
		else {
			Camera.main.orthographicSize = Mathf.Clamp (Camera.main.orthographicSize, orthographicSizeMin, orthographicSizeMax);
		}
	}
	
	// Update is called once per frame
	void LateUpdate () {
		
		transform.position = player.transform.position + offset ;
	}

	void FixedUpdate() {

		offset = Quaternion.AngleAxis (Input.GetAxis("Mouse X") * turnSpeed, Vector3.up) * offset;
		transform.position = player.transform.position + offset; 
		transform.LookAt(player.transform.position);
	}
		         
}
