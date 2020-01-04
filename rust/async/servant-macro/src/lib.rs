// -- lib.rs --

#[macro_use]
extern crate syn;
#[macro_use]
extern crate quote;
extern crate proc_macro;
extern crate proc_macro2;

// --

use proc_macro::TokenStream;
use syn::{FnArg, Ident, ItemTrait, ReturnType, Signature, TraitItem, TraitItemMethod};

// --

/// 定义带返回值方法的接口，接口中可以有多个带返回值的方法。
///
/// 在一个应用中，可以定义多个invoke接口，客户端调用这些接口的方法，请求服务端的服务，服务端
/// 通过方法的返回值，响应客户端的请求。
///
/// 在服务端会生成同名的trait，服务端要实现这个trait，在每个方法中提供服务。只有将每个实现添
/// 加到ServantRegister中，客户端才能请求该实现的服务。
///
/// 在客户端会生成后缀Proxy的struct，自动实现了该trait的方法，可以通过Terminal的proxy方
/// 法，生成这个proxy，调用接口的方法，向服务端请求服务。
///
/// # Notice
/// 接口中方法的第一个参数必须是&self或&mut self，因为接口在服务端都是按照对象提供服务的，每个接口
/// 可以有不同的实现类，每个类也可以有不同名字的对象分别提供服务。
///
/// # Example
/// ```
/// #[servant::invoke_interface]
/// pub trait Dog: Clone {
///     fn speak(&self, count: i32) -> String;
///     fn owner(&self) -> servant::Oid;
///     fn age(&mut self, i: u32) -> u32;
/// }
/// ```
#[cfg(feature = "invoke")]
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

    let output1 = if cfg!(any(feature = "adapter", feature = "terminal")) { quote! {
        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output2 = if cfg!(feature = "adapter") { quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods)*
        }
        pub struct #servant_ident<S> {
            name: String,
            entity: S,
        }
        impl<S> #servant_ident<S> {
            pub fn new(name: &str, entity: S) -> Self {
                Self { name: name.to_string(), entity }
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };

    let output3 = if cfg!(feature = "terminal") { quote!{
        #[derive(Clone)]
        pub struct #proxy_ident(servant::Oid, servant::Terminal);

        impl #proxy_ident {
            pub fn new(name: &str, t: &servant::Terminal) -> Self {
                let oid = servant::Oid::new(name, stringify!(#ident));
                Self(oid, t.clone())
            }

            #(
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

        }}
    } else {
        proc_macro2::TokenStream::new()
    };

    let output = quote! {
        #output1
        #output2
        #output3
    };
    output.into()
}

/// 定义不带返回值方法的接口，接口中可以有多个不带返回值的方法。
///
/// report接口是单向的，只能从客户端上报到服务端。
///
/// 在一个应用中，可以定义多个report接口，客户端调用这些接口的方法，向服务端发送报告，服务端
/// 通在实现接口的类中，使用这些报告信息。
///
/// 在服务端会生成同名的trait，服务端要实现这个trait，在每个方法中接收和处理客户端上报的信息。
/// 只有将每个实现添加到ServantRegister中，才能接收并处理客户端上报的信息。
///
/// 在客户端会生成后缀Proxy的struct，自动实现了该trait的方法，可以通过Terminal的proxy方
/// 法，生成这个proxy，调用接口的方法，向服务端请求服务。
///
/// # Notice
/// 接口中方法的第一个参数必须是&self或&mut self，因为接口在服务端都是按照对象提供服务的，每个接口
/// 可以有不同的实现类，每个类也可以有不同名字的对象分别提供服务。
///
/// # Example
/// ```
/// #[servant::report_interface]
/// pub trait Pusher {
///     fn f1(&self, count: i32);
///     fn f2(&self);
///     fn f3(&mut self, s: String);
/// }
/// ```
#[cfg(feature = "report")]
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

    let output1 = if cfg!(any(feature = "adapter", feature = "terminal")) { quote! {
        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output2 = if cfg!(feature = "adapter") { quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods_vec)*
        }
        pub struct #servant_ident<S> {
            name: String,
            entity: S,
        }
        impl<S> #servant_ident<S> {
            pub fn new(name: &str, entity: S) -> Self {
                Self { name: name.to_string(), entity }
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output3 = if cfg!(feature = "terminal") { quote!{
        #[derive(Clone)]
        pub struct #proxy_ident(servant::Oid, servant::Terminal);

        impl #proxy_ident {
            pub fn new(name: &str, t: &servant::Terminal) -> Self {
                let oid = servant::Oid::new(name, stringify!(#ident));
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };

    let output = quote! {
        #output1
        #output2
        #output3
    };
    output.into()
}

/// 定义query接口，query接口实际上就是一个invoke接口。
///
/// 在一个应用中，只可以有一个query接口。这个query接口向客户端提供本服务端的基本信息，比如，
/// 有那些提供服务的对象，有哪些接收报告的对象。
///
/// 在服务端会生成同名的trait，服务端要实现这个trait，在每个方法中提供服务。只有将每个实现添
/// 加到ServantRegister中，客户端才能请求该实现的服务。
///
/// 在客户端会生成后缀Proxy的struct，自动实现了该trait的方法，可以通过Terminal的proxy方
/// 法，生成这个proxy，调用接口的方法，向服务端请求服务。
///
/// 在servant中，缺省实现的query接口是Export，方便客户端查询服务端的信息。
///
/// 实际应用中，开发者可以定义自己的query接口。
///
/// # Notice
/// 接口中方法的第一个参数必须是&self或&mut self，因为接口在服务端都是按照对象提供服务的。
///
/// # Example
/// ```
/// #[servant_macro::query_interface]
/// pub trait Export {
///     fn export_servants(&self) -> Vec<Oid>;
///     fn export_report_servants(&self) -> Vec<Oid>;
///     fn shutdown(&self, passcode: usize);
/// }
/// ```
#[cfg(feature = "query")]
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

    let output1 = if cfg!(any(feature = "adapter", feature = "terminal")) { quote! {
        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output2 = if cfg!(feature = "adapter") { quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods)*
        }
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output3 = if cfg!(feature = "terminal") { quote!{
        #[derive(Clone)]
        pub struct #proxy_ident(servant::Terminal);

        impl #proxy_ident {
            pub fn new(t: &servant::Terminal) -> Self {
                Self(t.clone())
            }

            #(
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };

    let output = quote! {
        #output1
        #output2
        #output3
    };
    output.into()
}


/// 定义notify接口，notify接口中的方法不能有返回值。
///
/// notify接口是单向的，只是用来从服务器向每个连接的客户端发送通知。
///
/// 在一个应用中，只可以有一个notify接口。这个notify接口向客户端发布信息，比如，状态改变或定
/// 时性的通知信息。客户端在实现notify接口的ServantEntry中，接收并处理服务端的通知信息。
///
/// 在客户端会生成同名的trait，客户端要实现这个trait，在每个方法中接收并处理来自服务端的通知。
/// 只有将该实现添加到Terminal中，客户端才能收到并处理服务器端的通知。
///
/// 在服务端会生成后缀Notifier的struct，自动实现了该trait的方法。在服务端调用该struct的
/// instance()关联方法，得到该struct的一个引用，调用该引用的接口，向客户端发送通知。
///
/// # Notice
/// 接口中方法的第一个参数必须是&self或&mut self，因为接口在服务端都是按照对象提供服务的。
///
/// # Example
/// ```
/// #[servant::notify_interface]
/// pub trait StockNews {
///     fn f1(&self, count: i32);
///     fn f2(&self, msg: String);
///     fn f3(&mut self, count: usize, f: f64, b: Option<bool>, s: Vec<String>);
/// }
/// ```
#[cfg(feature = "notify")]
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

    let output1 = if cfg!(any(feature = "adapter", feature = "terminal")) { quote! {
        #[derive(serde::Serialize, serde::Deserialize)]
        enum #request_ident {
            #(#idents_camel_vec { #(#inputs_vec)* },)*
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output2 = if cfg!(feature = "terminal") { quote! {
        #( #attrs )*
        #vis #unsafety #auto_token #trait_token #ident #generics #colon_token #supertraits {
            #(#methods_vec)*
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };
    let output3 = if cfg!(feature = "adapter") { quote!{
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
        }}
    } else {
        proc_macro2::TokenStream::new()
    };

    let output = quote! {
        #output1
        #output2
        #output3
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
