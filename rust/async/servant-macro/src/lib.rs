// -- lib.rs --

#[macro_use]
extern crate syn;
#[macro_use]
extern crate quote;
extern crate proc_macro;

// --

use proc_macro::TokenStream;
use syn::{FnArg, Ident, ItemTrait, ReturnType, Signature, TraitItem, TraitItemMethod};

// --

#[proc_macro_attribute]
pub fn invoke_interface(_attr: TokenStream, input: TokenStream) -> TokenStream {
    // let derive_serde = parse_macro_input!(attr as DeriveSerde);
    let ItemTrait {
        attrs,
        vis,
        unsafety,
        auto_token,
        trait_token,
        ident,
        generics,
        colon_token,
        supertraits,
        // brace_token,
        items,
        ..
    } = parse_macro_input!(input as ItemTrait);

    let methods: Vec<TraitItemMethod> = items
        .iter()
        .map(|i| {
            if let TraitItem::Method(m) = i {
                Some(m)
            } else {
                None
            }
        })
        .filter(|i| i.is_some())
        .map(|x| x.unwrap().clone())
        .collect();
    let idents_collected: Vec<_> = methods
        .iter()
        .map(|x| {
            let TraitItemMethod {
                attrs,
                sig,
                default,
                semi_token,
            } = x;
            let Signature {
                constness,
                asyncness,
                unsafety,
                abi,
                fn_token,
                ident,
                generics,
                // paren_token,
                inputs,
                variadic,
                output,
                ..
            } = sig;

            let output_type = match output.clone() {
                ReturnType::Default => quote! {()},
                ReturnType::Type(_, t) => quote! {#t},
            };
            let ident_camel = Ident::new(&snake_to_camel(&ident.to_string()), ident.span());
            let args: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = &x.unwrap().pat;
                    quote! {#x,}
                })
                .collect();
            let inputs: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = x.unwrap();
                    quote! {#x,}
                })
                .collect();
            let method = quote! {
                #(#attrs)*
                #constness #asyncness #unsafety #abi #fn_token #ident #generics (
                    &mut self,
                    #(#inputs)* #variadic
                ) #output
                #default #semi_token
            };
            (ident, ident_camel, args, inputs, method, output_type)
        })
        .collect();
    let ident_vec: Vec<_> = idents_collected.iter().map(|i| i.0.clone()).collect();
    let idents_camel_vec: Vec<_> = idents_collected.iter().map(|i| i.1.clone()).collect();
    let args_vec: Vec<_> = idents_collected.iter().map(|i| i.2.clone()).collect();
    let inputs_vec: Vec<_> = idents_collected.iter().map(|i| i.3.clone()).collect();
    let _methods_vec: Vec<_> = idents_collected.iter().map(|i| i.4.clone()).collect();
    let output_vec: Vec<_> = idents_collected.iter().map(|i| i.5.clone()).collect();

    let request_ident = Ident::new(&format!("{}Request", ident), ident.span());
    let request_ident_vect: Vec<_> = idents_collected
        .iter()
        .map(|_| request_ident.clone())
        .collect();
    let servant_ident = Ident::new(&format!("{}Servant", ident), ident.span());
    let proxy_ident = Ident::new(&format!("{}Proxy", ident), ident.span());

    let output = quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods)*
        }

        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }

        // #[derive(Clone)]
        pub struct #servant_ident<S> {
            name: String,
            entity: S,
        }
        impl<S> #servant_ident<S> {
            pub fn new(name: String, entity: S) -> Self {
                Self { name, entity }
            }
        }
        impl<S> servant::Servant for #servant_ident<S>
        where
            S: #ident + 'static,
        {
            fn name(&self) -> &str {
                &self.name
            }
            fn category(&self) -> &'static str {
                stringify!(#ident)
            }
            fn serve(&mut self, req: Vec<u8>) -> Vec<u8> {
                let req: #request_ident = bincode::deserialize(&req).unwrap();
                let reps = match req {
                    #(
                        #request_ident_vect::#idents_camel_vec{ #(#args_vec)* } =>
                            bincode::serialize(&self.entity.#ident_vec(#(#args_vec)*)),
                    )*
                }
                .unwrap();
                reps
            }
        }

        // #[allow(unused)]
        #[derive(Clone)]
        pub struct #proxy_ident(servant::Oid, servant::Terminal);

        impl #proxy_ident {
            pub fn new(name: String, t: &servant::Terminal) -> Self {
                let oid = servant::Oid::new(name, stringify!(#ident).to_string());
                Self(oid, t.clone())
            }

            #(
            // #[allow(unused)]
            pub async fn #ident_vec(
                &mut self,
                #(#inputs_vec)*
            ) -> servant::ServantResult<#output_vec> {
                let request =  #request_ident_vect::#idents_camel_vec { #(#args_vec)* };
                let response = self
                    .1
                    .invoke(Some(self.0.clone()), bincode::serialize(&request).unwrap())
                    .await;
                response.map(|x| bincode::deserialize(&x).unwrap())
            }
            )*

        }
    };

    output.into()
}

#[proc_macro_attribute]
pub fn report_interface(_attr: TokenStream, input: TokenStream) -> TokenStream {
    let ItemTrait {
        attrs,
        vis,
        unsafety,
        auto_token,
        trait_token,
        ident,
        generics,
        colon_token,
        supertraits,
        // brace_token,
        items,
        ..
    } = parse_macro_input!(input as ItemTrait);

    let methods: Vec<TraitItemMethod> = items
        .iter()
        .map(|i| {
            if let TraitItem::Method(m) = i {
                Some(m)
            } else {
                None
            }
        })
        .filter(|i| i.is_some())
        .map(|x| x.unwrap().clone())
        .collect();
    let idents_collected: Vec<_> = methods
        .iter()
        .map(|x| {
            let TraitItemMethod {
                attrs,
                sig,
                default,
                semi_token,
            } = x;
            let Signature {
                constness,
                asyncness,
                unsafety,
                abi,
                fn_token,
                ident,
                generics,
                // paren_token,
                inputs,
                variadic,
                // output,
                ..
            } = sig;

            let ident_camel = Ident::new(&snake_to_camel(&ident.to_string()), ident.span());
            let args: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = &x.unwrap().pat;
                    quote! {#x,}
                })
                .collect();
            let inputs2: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = x.unwrap();
                    quote! {#x,}
                })
                .collect();
            let servant_method = quote! {
                #(#attrs)*
                #constness #asyncness #unsafety #abi #fn_token #ident #generics (
                    #inputs #variadic
                )
                #default #semi_token
            };
            (ident, ident_camel, args, inputs2, servant_method)
        })
        .collect();
    let ident_vec: Vec<_> = idents_collected.iter().map(|i| i.0.clone()).collect();
    let idents_camel_vec: Vec<_> = idents_collected.iter().map(|i| i.1.clone()).collect();
    let args_vec: Vec<_> = idents_collected.iter().map(|i| i.2.clone()).collect();
    let inputs_vec: Vec<_> = idents_collected.iter().map(|i| i.3.clone()).collect();
    let methods_vec: Vec<_> = idents_collected.iter().map(|i| i.4.clone()).collect();

    let request_ident = Ident::new(&format!("{}Request", ident), ident.span());
    let request_ident_vect: Vec<_> = idents_collected
        .iter()
        .map(|_| request_ident.clone())
        .collect();
    let servant_ident = Ident::new(&format!("{}ReportServant", ident), ident.span());
    let proxy_ident = Ident::new(&format!("{}ReportProxy", ident), ident.span());

    let output = quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods_vec)*
        }

        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }

        // #[derive(Clone)]
        pub struct #servant_ident<S> {
            name: String,
            entity: S,
        }
        impl<S> #servant_ident<S> {
            pub fn new(name: String, entity: S) -> Self {
                Self { name, entity }
            }
        }
        impl<S> servant::ReportServant for #servant_ident<S>
        where
            S: #ident + 'static,
        {
            fn name(&self) -> &str {
                &self.name
            }
            fn category(&self) -> &'static str {
                stringify!(#ident)
            }
            fn serve(&mut self, req: Vec<u8>) {
                let req: #request_ident = bincode::deserialize(&req).unwrap();
                match req {
                    #(
                        #request_ident_vect::#idents_camel_vec{ #(#args_vec)* } =>
                            self.entity.#ident_vec(#(#args_vec)*),
                    )*
                }
            }
        }

        // #[allow(unused)]
        #[derive(Clone)]
        pub struct #proxy_ident(servant::Oid, servant::Terminal);

        impl #proxy_ident {
            pub fn new(name: String, t: &servant::Terminal) -> Self {
                let oid = servant::Oid::new(name, stringify!(#ident).to_string());
                Self(oid, t.clone())
            }

            #(
            #[allow(unused)]
            pub async fn #ident_vec(
                &mut self,
                #(#inputs_vec)*
            ) -> servant::ServantResult<()> {
                let request =  #request_ident_vect::#idents_camel_vec { #(#args_vec)* };
                let response = self
                    .1
                    .report(self.0.clone(), bincode::serialize(&request).unwrap())
                    .await;
                response
            }
            )*

        }
    };

    output.into()
}

#[proc_macro_attribute]
pub fn query_interface(_attr: TokenStream, input: TokenStream) -> TokenStream {
    let ItemTrait {
        attrs,
        vis,
        unsafety,
        auto_token,
        trait_token,
        ident,
        generics,
        colon_token,
        supertraits,
        // brace_token,
        items,
        ..
    } = parse_macro_input!(input as ItemTrait);

    let methods: Vec<TraitItemMethod> = items
        .iter()
        .map(|i| {
            if let TraitItem::Method(m) = i {
                Some(m)
            } else {
                None
            }
        })
        .filter(|i| i.is_some())
        .map(|x| x.unwrap().clone())
        .collect();
    let idents_collected: Vec<_> = methods
        .iter()
        .map(|x| {
            let TraitItemMethod {
                attrs,
                sig,
                default,
                semi_token,
            } = x;
            let Signature {
                constness,
                asyncness,
                unsafety,
                abi,
                fn_token,
                ident,
                generics,
                // paren_token,
                inputs,
                variadic,
                output,
                ..
            } = sig;

            let output_type = match output.clone() {
                ReturnType::Default => quote! {()},
                ReturnType::Type(_, t) => quote! {#t},
            };
            let ident_camel = Ident::new(&snake_to_camel(&ident.to_string()), ident.span());
            let args: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = &x.unwrap().pat;
                    quote! {#x,}
                })
                .collect();
            let inputs: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = x.unwrap();
                    quote! {#x,}
                })
                .collect();
            let method = quote! {
                #(#attrs)*
                #constness #asyncness #unsafety #abi #fn_token #ident #generics (
                    &mut self,
                    #(#inputs)* #variadic
                ) #output
                #default #semi_token
            };
            (ident, ident_camel, args, inputs, method, output_type)
        })
        .collect();
    let ident_vec: Vec<_> = idents_collected.iter().map(|i| i.0.clone()).collect();
    let idents_camel_vec: Vec<_> = idents_collected.iter().map(|i| i.1.clone()).collect();
    let args_vec: Vec<_> = idents_collected.iter().map(|i| i.2.clone()).collect();
    let inputs_vec: Vec<_> = idents_collected.iter().map(|i| i.3.clone()).collect();
    let _methods_vec: Vec<_> = idents_collected.iter().map(|i| i.4.clone()).collect();
    let output_vec: Vec<_> = idents_collected.iter().map(|i| i.5.clone()).collect();

    let request_ident = Ident::new(&format!("{}Request", ident), ident.span());
    let request_ident_vect: Vec<_> = idents_collected
        .iter()
        .map(|_| request_ident.clone())
        .collect();
    let servant_ident = Ident::new(&format!("{}Servant", ident), ident.span());
    let proxy_ident = Ident::new(&format!("{}Proxy", ident), ident.span());

    let output = quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods)*
        }

        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }

        // #[derive(Clone)]
        pub struct #servant_ident<S> {
            entity: S,
        }
        impl<S> #servant_ident<S> {
            pub fn new(entity: S) -> Self {
                Self { entity }
            }
        }
        impl<S> servant::Servant for #servant_ident<S>
        where
            S: #ident + 'static,
        {
            fn name(&self) -> &str {
                ""
            }
            fn category(&self) -> &'static str {
                stringify!(#ident)
            }
            fn serve(&mut self, req: Vec<u8>) -> Vec<u8> {
                let req: #request_ident = bincode::deserialize(&req).unwrap();
                let reps = match req {
                    #(
                        #request_ident_vect::#idents_camel_vec{ #(#args_vec)* } =>
                            bincode::serialize(&self.entity.#ident_vec(#(#args_vec)*)),
                    )*
                }
                .unwrap();
                reps
            }
        }

        // #[allow(unused)]
        #[derive(Clone)]
        pub struct #proxy_ident(servant::Terminal);

        impl #proxy_ident {
            pub fn new(t: &servant::Terminal) -> Self {
                Self(t.clone())
            }

            #(
            // #[allow(unused)]
            pub async fn #ident_vec(
                &mut self,
                #(#inputs_vec)*
            ) -> servant::ServantResult<#output_vec> {
                let request =  #request_ident_vect::#idents_camel_vec { #(#args_vec)* };
                let response = self
                    .0
                    .invoke(None, bincode::serialize(&request).unwrap())
                    .await;
                response.map(|x| bincode::deserialize(&x).unwrap())
            }
            )*

        }
    };

    output.into()
}

#[proc_macro_attribute]
pub fn notify_interface(_attr: TokenStream, input: TokenStream) -> TokenStream {
    let ItemTrait {
        attrs,
        vis,
        unsafety,
        auto_token,
        trait_token,
        ident,
        generics,
        colon_token,
        supertraits,
        // brace_token,
        items,
        ..
    } = parse_macro_input!(input as ItemTrait);

    let methods: Vec<TraitItemMethod> = items
        .iter()
        .map(|i| {
            if let TraitItem::Method(m) = i {
                Some(m)
            } else {
                None
            }
        })
        .filter(|i| i.is_some())
        .map(|x| x.unwrap().clone())
        .collect();
    let idents_collected: Vec<_> = methods
        .iter()
        .map(|x| {
            let TraitItemMethod {
                attrs,
                sig,
                default,
                semi_token,
            } = x;
            let Signature {
                constness,
                asyncness,
                unsafety,
                abi,
                fn_token,
                ident,
                generics,
                // paren_token,
                inputs,
                variadic,
                // output,
                ..
            } = sig;

            let ident_camel = Ident::new(&snake_to_camel(&ident.to_string()), ident.span());
            let args: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = &x.unwrap().pat;
                    quote! {#x,}
                })
                .collect();
            let inputs2: Vec<_> = inputs
                .iter()
                .map(|i| {
                    if let FnArg::Typed(pat) = i {
                        Some(pat)
                    } else {
                        None
                    }
                })
                .filter(|i| i.is_some())
                .map(|x| {
                    let x = x.unwrap();
                    quote! {#x,}
                })
                .collect();
            let servant_method = quote! {
                #(#attrs)*
                #constness #asyncness #unsafety #abi #fn_token #ident #generics (
                    #inputs #variadic
                )
                #default #semi_token
            };
            (ident, ident_camel, args, inputs2, servant_method)
        })
        .collect();
        let ident_vec: Vec<_> = idents_collected.iter().map(|i| i.0.clone()).collect();
        let idents_camel_vec: Vec<_> = idents_collected.iter().map(|i| i.1.clone()).collect();
        let args_vec: Vec<_> = idents_collected.iter().map(|i| i.2.clone()).collect();
        let inputs_vec: Vec<_> = idents_collected.iter().map(|i| i.3.clone()).collect();
        let methods_vec: Vec<_> = idents_collected.iter().map(|i| i.4.clone()).collect();

        let request_ident = Ident::new(&format!("{}Request", ident), ident.span());
        let request_ident_vect: Vec<_> = idents_collected
            .iter()
            .map(|_| request_ident.clone())
            .collect();
        let receiver_ident = Ident::new(&format!("{}Receiver", ident), ident.span());
        let notifier_ident = Ident::new(&format!("{}Notifier", ident), ident.span());

        let output = quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods_vec)*
        }

        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }

        pub struct #receiver_ident<S> {
            entity: S,
        }
        impl<S> #receiver_ident<S> {
            pub fn new(entity: S) -> Self {
                Self { entity }
            }
        }
        impl<S> servant::NotifyServant for #receiver_ident<S>
        where
            S: #ident + 'static + Send,
        {
            fn serve(&mut self, req: Vec<u8>) {
                let req: #request_ident = bincode::deserialize(&req).unwrap();
                match req {
                    #(
                        #request_ident_vect::#idents_camel_vec{ #(#args_vec)* } =>
                            self.entity.#ident_vec(#(#args_vec)*),
                    )*
                }
            }
        }

        // --

        pub struct #notifier_ident(&'static servant::AdapterRegister);

        impl #notifier_ident {
            pub fn instance() -> &'static #notifier_ident {
                static mut NOTIFIER: Option<#notifier_ident> = None;
                static INIT: std::sync::Once = std::sync::Once::new();

                unsafe {
                    INIT.call_once(|| {
                        NOTIFIER = Some(#notifier_ident(servant::AdapterRegister::instance()));
                    });
                    NOTIFIER.as_ref().unwrap()
                }
            }

            #(
            pub async fn #ident_vec(
                &self,
                #(#inputs_vec)*
            )  {
                let request =  #request_ident_vect::#idents_camel_vec { #(#args_vec)* };
                self
                    .0
                    .send(bincode::serialize(&request).unwrap())
                    .await
            }
            )*

        }
    };

    output.into()
}

// --

fn snake_to_camel(ident_str: &str) -> String {
    let mut camel_ty = String::new();
    let chars = ident_str.chars();

    let mut last_char_was_underscore = true;
    for c in chars {
        match c {
            '_' => last_char_was_underscore = true,
            c if last_char_was_underscore => {
                camel_ty.extend(c.to_uppercase());
                last_char_was_underscore = false;
            }
            c => camel_ty.extend(c.to_lowercase()),
        }
    }

    camel_ty
}

#[allow(unused)]
fn snake_to_camel2(ident_str: &str) -> String {
    let mut camel_ty = String::new();
    let chars = ident_str.chars();

    let mut last_char_was_underscore = true;
    for c in chars {
        match c {
            '_' => last_char_was_underscore = true,
            c => camel_ty.push_str(&if last_char_was_underscore {
                last_char_was_underscore = false;
                c.to_uppercase().to_string()
            } else {
                c.to_lowercase().to_string()
            }),
        }
    }

    camel_ty
}

// --

#[cfg(test)]
mod tests {
    extern crate test_case;
    use super::*;
    use test_case::test_case;

    // --

    #[test_case("abc_def" => "AbcDef".to_string(); "basic")]
    #[test_case("abc_def_" => "AbcDef".to_string(); "suffix")]
    #[test_case("_abc_def"=> "AbcDef".to_string(); "prefix")]
    #[test_case("abc__def"=> "AbcDef".to_string(); "consecutive")]
    #[test_case("aBc_dEf"=> "AbcDef".to_string(); "middle")]
    #[test_case("__abc__def__" => "AbcDef".to_string(); "double middle")]
    fn test_snake_to_camel(ident_str: &str) -> String {
        // snake_to_camel(ident_str)
        snake_to_camel2(ident_str)
    }
}
