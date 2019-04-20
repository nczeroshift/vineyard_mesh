package org.nczeroshift.vineyard;

import org.nczeroshift.vineyard.payloads.VYPayloadFloat;

public class VYPacket {

    public enum Type{
        VOLTAGE('V', VYPayloadFloat.class),
        CURRENT('C', VYPayloadFloat.class),
        TEMPERATURE('T', VYPayloadFloat.class),
        HUMIDITY('H', VYPayloadFloat.class),
        PRESSURE('P', VYPayloadFloat.class),
        IO('I', VYPayloadFloat.class),

        ;


        public final byte code;
        public final Class payloadClass;

        Type(char code, Class payloadClass) {
            this.code = (byte)code;
            this.payloadClass = payloadClass;
        }
    }

    protected Type type = null;
    protected VYPayload payload = null;

    public VYPayload getPayload() {
        return payload;
    }

    public VYPacket setPayload(VYPayload payload) {
        this.payload = payload;
        return this;
    }

    public VYPacket() {
    }



    public VYPacket(Type type) {
        this.type = type;
    }

    public VYPacket setType(Type type){
        this.type = type;
        return this;
    }

    public Type getType() {
        return type;
    }


    @Override
    public String toString() {
        return "VYPacket{" +
                "type=" + type +
                (payload!=null ? ", payload=" + payload : "")+
                '}';
    }
}
