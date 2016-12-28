package fr.bowserf.testsoundsystem.spectrum;

import android.opengl.GLES20;
import android.opengl.GLES30;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;


/* package */
class Line {

    @SuppressWarnings("unused")
    private static final String TAG = "Line";

    // number of coordinates per vertex in this array
    private static final int COORDS_PER_VERTEX = 2;

    private static final int VERTEX_STRIDE = COORDS_PER_VERTEX * 4; // 4 bytes per vertex

    private static final int FLOAT_SIZE_IN_BYTE = 4;

    // Set color with red, green, blue and alpha (opacity) values
    private static float[] COLOR = {0.63671875f, 0.76953125f, 0.22265625f};

    // Vertex shader is used to render the vertices of a shape
    private static final String VERTEX_SHADER_CODE =
            "attribute vec4 vPosition;" +
            "void main() {" +
                "  gl_Position =  vPosition;" +
            "}";

    // Fragment shader is used to render the face of a shape with colors or textures
    private static final String FRAGMENT_SHADER_CODE =
            "precision mediump float;" +
            "uniform vec3 vColor;" +
            "uniform float width;" +
            "void main() {" +
                "float color = vColor.r * gl_FragCoord.x / width;" +
                "vec4 newColor = vec4(color, vColor.g, vColor.b, 1);" +
                "gl_FragColor = newColor;" +
            "}";

    private FloatBuffer mVertexBuffer;

    private int mPositionHandle;

    private int mVertexCount;

    private int mNumberDataForAverage;

    private float[] mCoordinates;

    private boolean mIsInitialized;

    private IntBuffer mIdVbo;
    private IntBuffer mIdVao;

    /* package */
    Line() {
        int vertexShader = loadShader(GLES20.GL_VERTEX_SHADER,
                VERTEX_SHADER_CODE);
        int fragmentShader = loadShader(GLES20.GL_FRAGMENT_SHADER,
                FRAGMENT_SHADER_CODE);

        // create empty OpenGL ES Program
        final int program = GLES20.glCreateProgram();

        // add the vertex shader to program
        GLES20.glAttachShader(program, vertexShader);

        // add the fragment shader to program
        GLES20.glAttachShader(program, fragmentShader);

        // creates OpenGL ES program executables
        GLES20.glLinkProgram(program);

        // Add program to OpenGL ES environment
        GLES20.glUseProgram(program);

        // disable depth to improve performances
        GLES20.glDepthMask(false);

        // get handle to vertex shader's vPosition member
        mPositionHandle = GLES20.glGetAttribLocation(program, "vPosition");

        // get handle to fragment shader's vColor member
        int colorHandle = GLES20.glGetUniformLocation(program, "vColor");

        // Set color for drawing the triangle
        GLES20.glUniform3fv(colorHandle, 1, COLOR, 0);

        // use to play a little bit with fragment shader to change color line
        final int widthHandle = GLES20.glGetUniformLocation(program, "width");
        GLES20.glUniform1f(widthHandle, 540);

        mIdVbo = IntBuffer.allocate(1);
        mIdVao = IntBuffer.allocate(1);
    }

    private int loadShader(int type, String shaderCode){
        // create a vertex shader type (GLES20.GL_VERTEX_SHADER)
        // or a fragment shader type (GLES20.GL_FRAGMENT_SHADER)
        int shader = GLES20.glCreateShader(type);

        // add the source code to the shader and compile it
        GLES20.glShaderSource(shader, shaderCode);
        GLES20.glCompileShader(shader);

        return shader;
    }

    /* package */
    void drawData(short[] data, int desiredNumberData){
        if(mVertexBuffer == null){
            mNumberDataForAverage = data.length / desiredNumberData;
            mCoordinates = new float[desiredNumberData * COORDS_PER_VERTEX * 2]; // *2 because start point (x,0) and high point (x, y)
            initVertexBuffer(mCoordinates.length);
        }

        generatePoints(data, desiredNumberData);

        mVertexBuffer.put(mCoordinates);
        // set the buffer to read the first coordinate
        mVertexBuffer.position(0);
    }

    private void initVertexBuffer(int vertexBufferSize){
        mVertexCount = vertexBufferSize / COORDS_PER_VERTEX;

        // initialize vertex byte buffer for shape coordinates
        final ByteBuffer bb = ByteBuffer.allocateDirect(
                // (number of coordinate values * 4 bytes per float)
                vertexBufferSize * 4);
        // use the device hardware's native byte order
        bb.order(ByteOrder.nativeOrder());

        // create a floating point buffer from the ByteBuffer
        mVertexBuffer = bb.asFloatBuffer();
        // add the coordinates to the FloatBuffer
    }

    /* package */
    void draw() {
        if(mVertexBuffer == null){
            return;
        }

        if(!mIsInitialized){
            mIsInitialized = true;

            if(GLES20.glIsBuffer(mIdVbo.get(0))){
                GLES20.glDeleteBuffers(1, mIdVbo);
            }

            if(GLES30.glIsVertexArray(mIdVao.get(0))){
                GLES30.glDeleteVertexArrays(1, mIdVao);
            }

            GLES20.glGenBuffers(1, mIdVbo);

            GLES30.glGenVertexArrays(1, mIdVao);

            GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, mIdVbo.get(0));

            // reserver mCoordinates.length * FLOAT_SIZE_IN_BYTE bytes inside GPU RAM.
            GLES20.glBufferData(GLES20.GL_ARRAY_BUFFER, mCoordinates.length * FLOAT_SIZE_IN_BYTE, null, GLES20.GL_STATIC_DRAW);

            // update data on GPU RAM
            GLES20.glBufferSubData(GLES20.GL_ARRAY_BUFFER, 0, mCoordinates.length * FLOAT_SIZE_IN_BYTE, mVertexBuffer);

            GLES30.glBindVertexArray(mIdVao.get(0));

            // Prepare the triangle coordinate data
            GLES20.glVertexAttribPointer(mPositionHandle,
                    COORDS_PER_VERTEX,
                    GLES20.GL_FLOAT,
                    false,
                    VERTEX_STRIDE,
                    0);

            // Enable a handle to the vertices
            GLES20.glEnableVertexAttribArray(mPositionHandle);

            GLES30.glBindVertexArray(0);

            GLES20.glBindBuffer(GLES20.GL_ARRAY_BUFFER, 0);
        }


        GLES30.glBindVertexArray(mIdVao.get(0));
        // Draw the triangle
        GLES20.glDrawArrays(GLES20.GL_LINES, 0, mVertexCount);

        GLES30.glBindVertexArray(0);
    }

    private void generatePoints(final short[] data, final int desiredNumberData){
        for(int i = 0 ; i < desiredNumberData ; i++){
            final float x = i / (float)(desiredNumberData/2) - 1;
            // bottom point
            mCoordinates[i * 4] = x;
            mCoordinates[i * 4 + 1] = 0;

            // vertical point
            mCoordinates[i * 4 + 2] = x;
            mCoordinates[i * 4 + 3] = computeAverageValue(data, i);
        }
    }

    private float computeAverageValue(final short[] data, final int index){
        float average = 0;
        for(int j = 0 ; j < mNumberDataForAverage ; j++){
            average += data[index * mNumberDataForAverage + j] / (float)(Short.MAX_VALUE);
        }
        return average / mNumberDataForAverage;
    }
}