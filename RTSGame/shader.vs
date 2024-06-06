#version 330

// Input vertex attributes (from vertex buffer)
in vec3 vertexPosition;
in vec3 vertexNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragNormal;
out vec3 fragPosition;

// Input uniform values
uniform mat4 mvp;
uniform mat4 modelMatrix;

void main()
{
    // Transform vertex position to world space
    vec4 worldPosition = modelMatrix * vec4(vertexPosition, 1.0);
    fragPosition = worldPosition.xyz;
    
    // Transform vertex normal to world space
    fragNormal = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
    
    // Calculate final vertex position
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
