
float2 pixelCoordToNCD(float2 pixel);

//struct vs_out vs_main(struct vs_in input);

cbuffer ConstantBuffer : register(b0)
{
    float width;
    float height;
    float time;
    float padding0;
};

    /* vertex attributes go here to input to the vertex shader */
struct vs_in
{
    uint vertexId : SV_VertexID;
};



/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out
{
    float4 pos : SV_POSITION; // required output of VS
    float4 colour : COLOR0;
};


float2 pixelCoordToNCD(float2 pixel)
{
    float2 ncd = ((pixel / float2(width, height)) - float2(0.5, 0.5)) * 2;
    return ncd;
}

vs_out vs_main(vs_in input)
{
    // TODO: What's the correct zero initialiser for hlsl 202x ?  {} and {0} do not work
    vs_out output = (vs_out) 0; // zero the memory first

    // Using the frame dimensions, a solitary fixed dimension square 100x100 in the middle of the screen
    // The pixel dimensions need to be scaled against the NDC space

    float2 center = float2(width / 2, height / 2);

    float2 topleft = pixelCoordToNCD(center + float2(-50, +50));
    float2 topright = pixelCoordToNCD(center + float2(+50, +50));

    float2 bottomleft = pixelCoordToNCD(center + float2(-50, -50));
    float2 bottomright = pixelCoordToNCD(center + float2(+50, -50));

    
    // Vertex lookup per instanceId
    float4 vertexData[] =
    {
        // Center triangle
        float4(0.5, -0.5, 0.5, 1.0),
        float4(-0.5, -0.5, 0.5, 1.0),
        float4(0.0, 0.5, 0.5, 1.0),
        
        // Top Left triangle
        float4(-0.9, 0.9, 0.5, 1.0),
        float4(-0.8, 0.9, 0.5, 1.0),
        float4(-0.9, 0.8, 0.5, 1.0),
        
        // Top Right triangle
        float4(0.9, 0.9, 0.5, 1.0),
        float4(0.9, 0.8, 0.5, 1.0),
        float4(0.8, 0.9, 0.5, 1.0),
        
        // Bottom Left triangle
        float4(-0.9, -0.8, 0.5, 1.0),
        float4(-0.8, -0.9, 0.5, 1.0),
        float4(-0.9, -0.9, 0.5, 1.0),
        
        // Bottom Right triangle
        float4(0.9, -0.8, 0.5, 1.0),
        float4(0.9, -0.9, 0.5, 1.0),
        float4(0.8, -0.9, 0.5, 1.0),
        
        // Centre Square
        float4(topleft, 0.5, 1.0),
        float4(topright, 0.5, 1.0),
        float4(bottomleft, 0.5, 1.0),
        float4(topright, 0.5, 1.0),
        float4(bottomright, 0.5, 1.0),
        float4(bottomleft, 0.5, 1.0)
    };
    
    output.pos = vertexData[input.vertexId];
       
    if (input.vertexId >= 15)
        output.colour = float4(1.0, 1.0, time, 1.0);
    else
        output.colour = clamp(output.pos, 0, 1);

    return output;
}