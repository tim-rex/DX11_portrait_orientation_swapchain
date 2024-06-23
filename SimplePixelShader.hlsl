
//struct ps_out ps_main(struct ps_in input);

struct ps_in
{
    float4 pos : SV_POSITION;
    linear float4 colour : COLOR0;
};

struct ps_out
{
    float4 colour : SV_TARGET;
};

ps_out ps_main(ps_in input)
{
    ps_out output;
    output.colour = input.colour;
    //output.colour = float4(1.0, 1.0, 0.0, 1.0);
    return output;
}