
//import {mat4} from '../libs/gl-matrix.js';

export class Visual
{
	constructor(canvas)
	{
		// canvas.width = canvas.clientWidth;
		// canvas.height = canvas.clientHeight;

		const gl = canvas.getContext("webgl2"); 
		
		this.canvas = canvas;
		this.gl = gl;

		if (this.gl)
		{

			// Vertex shader program

			const vsSource = `#version 300 es
				in vec4 in_position;
				// uniform mat4 uModelViewMatrix;
				// uniform mat4 uProjectionMatrix;
				void main() 
				{
					//gl_Position = uProjectionMatrix * uModelViewMatrix * aVertexPosition;
					gl_Position = in_position;
				}
			`;

			// Fragment shader program

			const fsSource = `#version 300 es
				precision mediump float;

				out vec4 out_color;

				void main() 
				{
					out_color = vec4(1, 0, 0.5, 1);
				}
			`;

			const vs = compileShader(gl, vsSource, gl.VERTEX_SHADER);
			const fs = compileShader(gl, fsSource, gl.FRAGMENT_SHADER);
			const program = createProgram(gl, vs, fs);

			const primitive = createPrimitive(gl,
			{
				type: gl.TRIANGLES,
				count: 3,
				program: program,
				input:
				{
					in_position: 
					{
						size: 2, type: gl.FLOAT,
						buffer: new Float32Array(
						[
							0, 0,
							0, 0.5,
							0.7, 0,
						])
					}
				}
			});

			(function render_loop()
			{
				resize(gl, canvas);
				render(gl, [primitive]);
				window.requestAnimationFrame(render_loop);
			})();
		}
	}
}

function resize(gl, canvas)
{
	// Lookup the size the browser is displaying the canvas.
	const displayWidth  = canvas.clientWidth;
	const displayHeight = canvas.clientHeight;

	// Check if the canvas is not the same size.
	if (canvas.width !== displayWidth || canvas.height !== displayHeight) 
	{
		// Make the canvas the same size
		canvas.width  = displayWidth;
		canvas.height = displayHeight;
		gl.viewport(0, 0, canvas.width, canvas.height);
	}	
}

function render(gl, primitives)
{
	gl.clearColor(0.0, 0.0, 0.0, 0.0);
	gl.clearDepth(1.0);                 // Clear everything
	gl.enable(gl.DEPTH_TEST);           // Enable depth testing
	gl.depthFunc(gl.LEQUAL);            // Near things obscure far things
  
	// Clear the canvas before we start drawing on it.
  
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);


	for (let primitive of primitives)
	{
		gl.useProgram(primitive.program);
		gl.bindVertexArray(primitive.vao);
		gl.drawArrays(primitive.type, 0, primitive.count);
	}

	gl.useProgram(null);
	gl.bindVertexArray(null);
}

function compileShader(gl, shaderSource, shaderType) 
{
	// Create the shader object
	const shader = gl.createShader(shaderType);
   
	// Set the shader source code.
	gl.shaderSource(shader, shaderSource);
   
	// Compile the shader
	gl.compileShader(shader);
   
	// Check if it compiled
	if (gl.getShaderParameter(shader, gl.COMPILE_STATUS)) 
	{
		return shader;
	}
	else
	{
		// Something went wrong during compilation; get the error
		console.error(`Could not compile shader: ${gl.getShaderInfoLog(shader)}`);
		gl.deleteShader(shader);
		return null;
	}
}

function createProgram(gl, vertexShader, fragmentShader) 
{
	// create a program.
	const program = gl.createProgram();
   
	// attach the shaders.
	gl.attachShader(program, vertexShader);
	gl.attachShader(program, fragmentShader);
   
	// link the program.
	gl.linkProgram(program);
   
	// Check if it linked.
	if (gl.getProgramParameter(program, gl.LINK_STATUS)) 
	{
		return program;
	}
	else
	{
		// something went wrong with the link
		console.error(`program filed to link: ${gl.getProgramInfoLog(program)}`);
		gl.deleteProgram(program);
		return null;
	}
}

function createPrimitive(gl, params)
{
	const vao = gl.createVertexArray();
	gl.bindVertexArray(vao);

	for (let attr in params.input)
	{
		const info = params.input[attr];
		const attrLoc = gl.getAttribLocation(params.program, attr);

		const buffer = gl.createBuffer();		
		gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
		gl.bufferData(gl.ARRAY_BUFFER, info.buffer, gl.STATIC_DRAW);

		gl.enableVertexAttribArray(attrLoc);
		gl.vertexAttribPointer(attrLoc, info.size, info.type, false, 0, 0);

		gl.bindBuffer(null);
	}

	gl.bindVertexArray(null);

	const primitive = new Primitive;
	primitive.type = params.type;
	primitive.count = params.count;
	primitive.vao = vao;
	primitive.program = params.program;
	return primitive;
}

class Primitive
{
	constructor()
	{
		this.type = 0,
		this.count = 0;
		this.vao = null;
		this.program = null;
	}
}