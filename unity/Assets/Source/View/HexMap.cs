using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class HexMap : MonoBehaviour
{
	public uint Width = 1;
	public uint Height = 1;

	public float HexH = 0.25f;

	public float HexV = 0.4f;

	public bool StartLower = false;

	Mesh HexMesh;


	void Awake()
	{
		GetComponent<MeshFilter>().mesh = HexMesh = MakeMesh();
		// hexMesh.name = "Hex Mesh";
		// vertices = new List<Vector3>();
		// triangles = new List<int>();

		// vertices.Add(new Vector3(0, 1));
		// vertices.Add(new Vector3(1, 0));
		// vertices.Add(new Vector3(0, 0));

		// triangles.Add(0);
		// triangles.Add(1);
		// triangles.Add(2);

		// hexMesh.vertices = vertices.ToArray();
		// hexMesh.triangles = triangles.ToArray();
		// hexMesh.RecalculateNormals();
	}

	// Use this for initialization
	void Start ()
	{
		
	}
	
	// Update is called once per frame
	void Update ()
	{
		
	}

	Mesh MakeMesh()
	{
		
		var total_width = (Width * 3 + 1) * HexH;
		var total_height = (Height * 2 + 1) * HexV;

		var offset = new Vector3(-total_width * 0.5f, -total_height * 0.5f);

		List<Vector3> vertices = new List<Vector3>();
		List<int> triangles = new List<int>();
		List<Color> colors = new List<Color>();

		for (int x = 0; x < Width; x++)
		{
			var yd = (StartLower ? (x % 2) : (1 - (x % 2)));
			
			for (int y = 0; y < Height; y++)
			{
				var start = vertices.Count;

				vertices.Add(offset + new Vector3((3 * x + 1) * HexH, (yd + y * 2) * HexV));
				vertices.Add(offset + new Vector3((3 * x + 3) * HexH, (yd + y * 2) * HexV));
				vertices.Add(offset + new Vector3((3 * x + 4) * HexH, (yd + y * 2 + 1) * HexV));
				vertices.Add(offset + new Vector3((3 * x + 3) * HexH, (yd + y * 2 + 2) * HexV));
				vertices.Add(offset + new Vector3((3 * x + 1) * HexH, (yd + y * 2 + 2) * HexV));
				vertices.Add(offset + new Vector3((3 * x) * HexH, (yd + y * 2 + 1) * HexV));

				triangles.Add(start + 0);
				triangles.Add(start + 3);
				triangles.Add(start + 1);

				triangles.Add(start + 0);
				triangles.Add(start + 4);
				triangles.Add(start + 3);

				triangles.Add(start + 1);
				triangles.Add(start + 3);
				triangles.Add(start + 2);

				triangles.Add(start + 0);
				triangles.Add(start + 5);
				triangles.Add(start + 4);

				var color = Random.ColorHSV();
				colors.AddRange(Enumerable.Repeat(color, 6));
			}
		}

		var mesh = new Mesh();
		mesh.name = "Hex Mesh";
		mesh.vertices = vertices.ToArray();
		mesh.triangles = triangles.ToArray();
		mesh.colors = colors.ToArray();
		mesh.RecalculateNormals();
		return mesh;
	}
}
